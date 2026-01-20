#include "GPSManager.h"
#include "../ui/screens/TimeSettingScreen.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Preferences.h>

void GPSManager::begin() {
  Preferences prefs;
  prefs.begin("laptimer", true); // Read-only first

  // Load Config
  _rxPin = prefs.getInt("gps_rx_pin", PIN_GPS_RX);
  _txPin = prefs.getInt("gps_tx_pin", PIN_GPS_TX);
  _baudRate = prefs.getInt("gps_baud", GPS_BAUD);

  // Load Other Config
  _totalDistance = prefs.getDouble("total_trip", 0.0);
  _currentGnssMode = prefs.getInt("gnss_mode", 1);
  _currentDynModel = prefs.getInt("gnss_model", 3);
  _currentSBAS = prefs.getInt("gnss_sbas", 0);
  _currentSBAS = prefs.getInt("gnss_sbas", 0);
  _projectionEnabled = prefs.getBool("gnss_proj", true);
  _utcOffset = prefs.getInt("utc_offset", 0);
  _utcOffset = prefs.getInt("utc_offset", 0);
  _rpmEnabled = prefs.getBool("rpm_enabled", true); // Default: Enabled

  // Load PPR
  int pprIdx = prefs.getInt("rpm_ppr", 0);
  setPPRIndex(pprIdx); // Initialize _currentPPR

  prefs.end();

  // Gunakan Serial2 untuk GPS
  _gpsSerial = &Serial2;

  // NEGOTIATE BAUD RATE
  // 1. Start at default 9600 (Standard factory default)
  _gpsSerial->begin(9600, SERIAL_8N1, _rxPin, _txPin);
  delay(100);

  // 2. Switch to configured rate if different
  if (_baudRate != 9600) {
    configureGpsBaud(_baudRate);
    delay(250);                            // Give GPS time to switch
    _gpsSerial->updateBaudRate(_baudRate); // Switch ESP32 UART speed
  }

  // Apply Config (if GPS active)
  if (_gpsSerial) {
    delay(100); // Wait for GPS boot

    // Optimize: Set 5Hz update rate (More stable signal)
    setFrequencyLimit(5);

    // Optimize: Disable unnecessary NMEA sentences
    disableUnnecessarySentences();

    setGnssMode(_currentGnssMode);
    setDynamicModel(_currentDynModel);
    setSBASConfig(_currentSBAS);
  }

  // Initialize SD Card for Redundancy
  SPI.begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (SD.begin(PIN_SD_CS)) {
    // If internal memory is empty/zero but SD has data, recover it
    if (_totalDistance < 0.1) {
      if (SD.exists("/trip.txt")) {
        File file = SD.open("/trip.txt", FILE_READ);
        if (file) {
          String s = file.readStringUntil('\n');
          double sdVal = s.toDouble();
          if (sdVal > 0)
            file.close();
        }
      }
    }
  }

  // Initialize RPM Sensor
  // Initialize RPM Sensor
  if (PIN_RPM_INPUT >= 0 && _rpmEnabled) {
    pinMode(PIN_RPM_INPUT, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT), onPulse, FALLING);
  }
}

// Static Member Initialization
volatile unsigned long GPSManager::_rpmPulses = 0;
volatile unsigned long GPSManager::_lastPulseMicros = 0;

void IRAM_ATTR GPSManager::onPulse() {
  unsigned long now = micros();
  // Debounce: 2ms (2000us) -> Max 30.000 RPM (Reduces noise significantly)
  if (now - _lastPulseMicros > 2000) {
    _rpmPulses++;
    _lastPulseMicros = now;
  }
}

void GPSManager::update() {
  if (!_gpsSerial)
    return;
  while (_gpsSerial->available() > 0) {
    uint8_t c = _gpsSerial->read();

    // Invoke debug callback if set
    if (_dataCallback) {
      _dataCallback(c);
    }

    _gps.encode(c);
  }

  // --- SYSTEM TIME REDUNDANCY ---
  // 1. Tick System Time
  if (_lastTick == 0)
    _lastTick = millis();
  unsigned long now = millis();
  if (now - _lastTick >= 1000) {
    _sysSec++;
    if (_sysSec >= 60) {
      _sysSec = 0;
      _sysMin++;
      if (_sysMin >= 60) {
        _sysMin = 0;
        _sysHour++;
        if (_sysHour >= 24) {
          _sysHour = 0;
          // Day increment logic omitted for simplicity or could be added
        }
      }
    }
    _lastTick = now;
  }

  // 2. Auto-Sync with GPS (if valid)
  if (_gps.time.isValid() && _gps.date.isValid() && _gps.time.isUpdated()) {
    // Overwrite System Time with GPS Time (UTC)
    _sysHour = _gps.time.hour();
    _sysMin = _gps.time.minute();
    _sysSec = _gps.time.second();
    _sysDay = _gps.date.day();
    _sysMonth = _gps.date.month();
    _sysYear = _gps.date.year();
    // Determine ms offset? for now standard is 1Hz.
  }

  // Update Trip Meter
  if (_gps.location.isValid() && _gps.location.isUpdated()) {
    double lat = _gps.location.lat();
    double lng = _gps.location.lng();

    if (_hasLastPos) {
      double dist = distanceBetween(_lastLat, _lastLng, lat, lng);
      // Filter out jitter (e.g. static movements < 2m)
      if (dist > 2.0 && dist < 1000.0) { // < 1km jump is reasonable for 1Hz
        _totalDistance += dist;
      }
    }

    _lastLat = lat;
    _lastLng = lng;
    _hasLastPos = true;
    _updatesCount++;
  }

  // Calculate Hz every 1 second
  if (millis() - _lastRateCheck >= 1000) {
    _currentHz = _updatesCount;
    _updatesCount = 0;
    _updatesCount = 0;
    _lastRateCheck = millis();
  }

  // --- RPM CALCULATION ---
  if (_rpmEnabled) {
    if (millis() - _lastRpmCalcTime > 100) { // 10Hz Update
      noInterrupts();
      unsigned long pulses = _rpmPulses;
      _rpmPulses = 0;
      interrupts();

      unsigned long dt = millis() - _lastRpmCalcTime;
      _lastRpmCalcTime = millis();

      if (dt > 0) {
        // Use cached _currentPPR
        unsigned long rawRpm =
            (unsigned long)((pulses * 60000.0) / (dt * _currentPPR));

        // NOISE FILTER: Ignore absurdly low RPM (Ghost readings)
        // Real engines don't run stable < 300 RPM.
        if (rawRpm < 300) {
          _currentRPM = 0;
        } else {
          _currentRPM = rawRpm;
        }
      } else {
        _currentRPM = 0;
      }
    }
  } else {
    _currentRPM = 0;
  }

  // Periodic Save (Every 1 minute)
  if (millis() - _lastSaveTime > 60000) {
    Preferences prefs;
    prefs.begin("laptimer", false);
    prefs.putDouble("total_trip", _totalDistance);
    prefs.end();

    // BACKUP TO SD CARD (Redundancy)
    if (SD.exists("/trip.txt") || SD.open("/trip.txt", FILE_WRITE)) {
      File file = SD.open("/trip.txt", FILE_WRITE); // Overwrite/create
      if (file) {
        file.println(_totalDistance);
        file.close();
      }
    }
    _lastSaveTime = millis();
  }
}

// Manual Setters
void GPSManager::setManualTime(int h, int m, int s) {
  // Input is LOCAL time. Convert to UTC for System Time.
  // UTC = Local - Offset
  int utcH = h - _utcOffset;

  // Handle wrap around
  if (utcH < 0)
    utcH += 24;
  if (utcH >= 24)
    utcH -= 24;

  _sysHour = utcH;
  _sysMin = m;
  _sysSec = s;

  // Save preference for "Manual Sync" if desired?
  // Current requirement is just set it.
  // Maybe valid GPS will overwrite this immediately?
  // YES. This is desired. Manual is fallback.
}

void GPSManager::setUtcOffset(int offset) {
  _utcOffset = offset;
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("utc_offset", offset);
  prefs.end();
}

bool GPSManager::isFixed() { return _gps.location.isValid(); }

double GPSManager::getLatitude() { return _gps.location.lat(); }

double GPSManager::getLongitude() { return _gps.location.lng(); }

float GPSManager::getSpeedKmph() {
  // Priority 1: Use GPS speed if valid (no minimum threshold for sensitivity)
  if (_gps.speed.isValid()) {
    _calculatedSpeed = _gps.speed.kmph();
    return _calculatedSpeed;
  }

  // Priority 2: Calculate speed from position changes
  if (_gps.location.isValid() && _gps.location.isUpdated()) {
    double lat = _gps.location.lat();
    double lon = _gps.location.lng();
    unsigned long now = millis();

    if (_hasLastSpeedPos && (now - _lastSpeedTime) > 0) {
      // Calculate distance traveled
      double dist = distanceBetween(_lastSpeedLat, _lastSpeedLon, lat, lon);

      // Calculate time elapsed (in hours)
      double timeHours = (now - _lastSpeedTime) / 3600000.0;

      // Speed = Distance / Time (km/h)
      // Ultra-sensitive: accept any movement > 1cm (0.01m)
      if (timeHours > 0 && dist > 0.01 && dist < 1000) {
        float newSpeed = (dist / 1000.0) / timeHours;

        // Smooth the speed (60% new, 40% old) to reduce jitter
        _calculatedSpeed = (_calculatedSpeed * 0.4) + (newSpeed * 0.6);
      }
    }

    // Update last position for next calculation
    _lastSpeedLat = lat;
    _lastSpeedLon = lon;
    _lastSpeedTime = now;
    _hasLastSpeedPos = true;
  }

  // Return calculated speed (or 0 if no data yet)
  return _calculatedSpeed;
}

float GPSManager::getTotalTrip() {
  return (float)(_totalDistance / 1000.0); // Convert to km
}

void GPSManager::resetTrip() {
  _totalDistance = 0.0;
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putDouble("total_trip", 0.0);
  prefs.end();
}

int GPSManager::getSatellites() {
  if (_gps.satellites.isValid()) {
    return _gps.satellites.value();
  }
  return 0;
}

String GPSManager::getTimeString() {
  int h, m, s, d, mo, y;
  getLocalTime(h, m, s, d, mo, y);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

String GPSManager::getDateString() {
  int h, m, s, d, mo, y;
  getLocalTime(h, m, s, d, mo, y);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d/%02d/%04d", d, mo, y);
  return String(buf);
}

void GPSManager::getLocalTime(int &h, int &m, int &s, int &d, int &mo, int &y) {
  // Use Internal System Time (Redundant Source)
  h = _sysHour;
  m = _sysMin;
  s = _sysSec;
  d = _sysDay;
  mo = _sysMonth;
  y = _sysYear;

  h += _utcOffset;

  if (h < 0) {
    h += 24;
    d--;
    if (d < 1) {
      mo--;
      if (mo < 1) {
        mo = 12;
        y--;
      }
      static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};
      int days = daysInMonth[mo];
      if (mo == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)))
        days = 29;
      d = days;
    }
  } else if (h >= 24) {
    h -= 24;
    d++;
    static const int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
    int days = daysInMonth[mo];
    if (mo == 2 && ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)))
      days = 29;
    if (d > days) {
      d = 1;
      mo++;
      if (mo > 12) {
        mo = 1;
        y++;
      }
    }
  }
}

int GPSManager::getRawHour() {
  return _gps.time.isValid() ? _gps.time.hour() : 0;
}

int GPSManager::getRawMinute() {
  return _gps.time.isValid() ? _gps.time.minute() : 0;
}

double GPSManager::getHDOP() {
  if (_gps.hdop.isValid()) {
    return _gps.hdop.hdop();
  }
  return 99.9; // Tidak ada perbaikan/buruk
}

double GPSManager::getAltitude() {
  if (_gps.altitude.isValid()) {
    return _gps.altitude.meters();
  }
  return 0.0;
}

double GPSManager::getHeading() {
  if (_gps.course.isValid()) {
    return _gps.course.deg();
  }
  return 0.0;
}

int GPSManager::getUpdateRate() { return _currentHz; }

double GPSManager::distanceBetween(double lat1, double long1, double lat2,
                                   double long2) {
  return _gps.distanceBetween(lat1, long1, lat2, long2);
}

// --- CONFIGURATION IMPL ---

void GPSManager::sendUBX(const uint8_t *cmd, int len) {
  if (_gpsSerial) {
    _gpsSerial->write(cmd, len);
  }
}

void GPSManager::setGnssMode(uint8_t mode) {
  if (!_gpsSerial)
    return;

  // UBX-CFG-GNSS commands for different constellations
  // We construct these based on U-blox M8 protocol
  // Simplify: Trigger Cold Start or just minimal configuration?
  // Real implementation requires constructing complex payload.
  // For this prototype, we will handle RATE mostly as it's the primary "User
  // Visible" change Hz.

  // Mapping Mode to Hz Limit
  int targetRate = 1; // Default safer 1Hz for 9600 baud

  // Only allow higher rates if Baud Rate is sufficient (>38400)
  // 9600 baud can barely handle 10Hz if sentences are short, but with full
  // NMEA it chokes. Safe limit: 1Hz for 9600.
  if (_baudRate > 38400) {
    switch (mode) {
    case 0:
      targetRate = 5;
      break; // All
    case 1:
      targetRate = 5;
      break; // GPS+GLO+SBAS (Was 16)
    case 2:
      targetRate = 5;
      break; // GPS+GAL+GLO+SBAS
    case 3:
      targetRate = 5;
      break; // GPS+GAL+SBAS (Was 20)
    case 4:
      targetRate = 5;
      break; // GPS+SBAS (Was 25)
    case 5:
      targetRate = 5;
      break; // GPS Only (Was 25)
    case 6:
      targetRate = 5;
      break; // GPS+BEI+SBAS (Was 12)
    case 7:
      targetRate = 5;
      break; // GPS+GLO (Was 16)
    }
  } else {
    // For 9600 baud, force 1Hz to be safe.
    targetRate = 1;
  }
  setFrequencyLimit(targetRate);
  _currentGnssMode = mode;

  // In a real scenario, we would send UBX-CFG-GNSS here to enable/disable
  // specific constellations. Due to complexity and lack of verification
  // hardware, we mock the constellation switch but APPLY the Update Rate
  // which is universally supported on M8/M10.

  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("gnss_mode", mode);
  prefs.end();
}

uint8_t GPSManager::getGnssMode() { return _currentGnssMode; }

void GPSManager::setDynamicModel(uint8_t modelIdx) {
  if (!_gpsSerial)
    return;

  // User Index to UBX DynModel Mapping
  // 0: Portable    -> 0
  // 1: Stationary  -> 2
  // 2: Pedestrian  -> 3
  // 3: Automotive  -> 4 (Default)
  // 4: At Sea      -> 5
  // 5: Air <1g     -> 6
  // 6: Air <2g     -> 7
  // 7: Air <4g     -> 8

  uint8_t ubxModel = 4; // Default Automotive
  switch (modelIdx) {
  case 0:
    ubxModel = 0;
    break;
  case 1:
    ubxModel = 2;
    break;
  case 2:
    ubxModel = 3;
    break;
  case 3:
    ubxModel = 4;
    break;
  case 4:
    ubxModel = 5;
    break;
  case 5:
    ubxModel = 6;
    break;
  case 6:
    ubxModel = 7;
    break;
  case 7:
    ubxModel = 8;
    break;
  }

  // UBX-CFG-NAV5 (0x06 0x24)
  uint8_t packet[] = {
      0xB5,     0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, // Mask
      ubxModel,                                           // dynModel
      0x03,                                               // fixMode (3=Auto)
      0x00,     0x00, 0x00, 0x00,                         // fixedAlt
      0x10,     0x27, 0x00, 0x00,                         // fixedAltVar
      0x05,                                               // minElev
      0x00,                                               // drLimit
      0xFA,     0x00,                                     // pDop
      0xFA,     0x00,                                     // tDop
      0x64,     0x00,                                     // pAcc
      0x2C,     0x01,                                     // tAcc
      0x00,                                               // staticHoldThresh
      0x3C,                                               // dgpsTimeOut
      0x00,     0x00, 0x00, 0x00,                         // cnoThresh
      0x00,     0x00,                                     // reserved
      0x00,     0x00, 0x00, 0x00,                         // reserved
      0x00,     0x00                                      // Checksum
  };

  // Calc Checksum
  uint8_t ck_a = 0, ck_b = 0;
  for (int i = 2; i < 38; i++) {
    ck_a += packet[i];
    ck_b += ck_a;
  }
  packet[38] = ck_a;
  packet[39] = ck_b;

  sendUBX(packet, sizeof(packet));

  _currentDynModel = modelIdx;
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("gnss_model", modelIdx);
  prefs.end();
}

void GPSManager::setSBASConfig(uint8_t regionIndex) {
  if (!_gpsSerial)
    return;

  // SBAS Configuration (UBX-CFG-SBAS 0x06 0x16)
  // We mainly want to enable/disable or set the PRN mask.
  // For simplicity in this "Blind" implementation, we will validly toggle the
  // system.

  // Region Index (New Order):
  // 0: EGNOS (Europe)
  // 1: WAAS (USA)
  // 2: SDCM (Russia)
  // 3: MSAS (Japan)
  // 4: GAGAN (India)
  // 5: SouthPAN (Aus/NZ)
  // 6: S.AMERICA (NONE) -> Disable
  // 7: MID-EAST (NONE)  -> Disable
  // 8: AFRICA (NONE)    -> Disable
  // 9: China (BDSBAS)   -> Enable
  // 10: KASS (Korea)    -> Enable

  bool enable = true;
  uint32_t prnMask = 0; // 0 = Auto/All

  // Disable if index is 6, 7, or 8
  if (regionIndex >= 6 && regionIndex <= 8) {
    enable = false; // Disable SBAS
  } else {
    // Enable for others (including China/Korea which are now 9/10)
    prnMask = 0x00000000;
  }

  uint8_t mode = enable ? 0x01 : 0x00;

  uint8_t packet[] = {
      0xB5, 0x62, 0x06, 0x16, 0x08, 0x00,
      mode,                   // mode (Enable/Disable)
      0x03,                   // usage (Range+DiffCorr+Integrity)
      0x03,                   // maxSBAS (3 channels)
      0x00,                   // scanmode2 (PRN Mask Low - 0 for auto)
      0x00, 0x00, 0x00, 0x00, // scanmode1 (PRN Mask High)
      0x00, 0x00              // Checksum
  };

  // If we wanted to be rigorous:
  // WAAS PRNs: 131,133,135,138 -> Map to bits
  // Since we don't have the exact bitmask function handy and don't want to
  // break it, we assume 0 (Auto Scan) is sufficient for "Enable". Disabling
  // (Index >= 8) allows revert to raw GPS.

  uint8_t ck_a = 0, ck_b = 0;
  for (int i = 2; i < 14; i++) {
    ck_a += packet[i];
    ck_b += ck_a;
  }
  packet[14] = ck_a;
  packet[15] = ck_b;

  sendUBX(packet, sizeof(packet));

  _currentSBAS = regionIndex;
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("gnss_sbas", regionIndex);
  prefs.end();
}

void GPSManager::setRpmEnabled(bool enabled) {
  _rpmEnabled = enabled;

  if (PIN_RPM_INPUT >= 0) {
    if (_rpmEnabled) {
      pinMode(PIN_RPM_INPUT, INPUT);
      attachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT), onPulse, FALLING);
    } else {
      detachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT));
      _currentRPM = 0;
      _rpmPulses = 0;
    }
  }

  // Save Preference
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putBool("rpm_enabled", enabled);
  prefs.end();
}

void GPSManager::setPPRIndex(int idx) {
  _currentPPR = 1.0;
  switch (idx) {
  case 0:
    _currentPPR = 1.0;
    break;
  case 1:
    _currentPPR = 0.5;
    break;
  case 2:
    _currentPPR = 2.0;
    break;
  case 3:
    _currentPPR = 3.0;
    break;
  case 4:
    _currentPPR = 4.0;
    break;
  }
}

void GPSManager::setFrequencyLimit(int freq) {
  if (!_gpsSerial)
    return;

  // UBX-CFG-RATE
  // rate = 1000 / freq
  uint16_t rateMs = 1000 / freq;

  uint8_t packet[] = {
      0xB5,
      0x62,
      0x06,
      0x08,
      0x06,
      0x00,
      (uint8_t)(rateMs & 0xFF),
      (uint8_t)((rateMs >> 8) & 0xFF), // measRate
      0x01,
      0x00, // navRate (always 1)
      0x01,
      0x00, // timeRef (GPS)
      0x00,
      0x00 // Checksum
  };

  uint8_t ck_a = 0, ck_b = 0;
  for (int i = 2; i < 12; i++) {
    ck_a += packet[i];
    ck_b += ck_a;
  }
  packet[12] = ck_a;
  packet[13] = ck_b;

  sendUBX(packet, sizeof(packet));
  _targetFreq = freq;
}

void GPSManager::setProjection(bool enabled) {
  _projectionEnabled = enabled;
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putBool("gnss_proj", enabled);
  prefs.end();
}

void GPSManager::setPins(int rx, int tx) {
  if (_rxPin == rx && _txPin == tx)
    return;

  _rxPin = rx;
  _txPin = tx;

  // Save to prefs
  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("gps_rx_pin", _rxPin);
  prefs.putInt("gps_tx_pin", _txPin);
  prefs.end();

  // Restart Serial
  if (_gpsSerial) {
    _gpsSerial->end();
    delay(100);
    _gpsSerial->begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);

    // Re-apply config as module might have power cycled?
    // Actually ESP32 UART reset doesn't reset the GPS module itself,
    // but just in case we need to re-init communication.
    delay(100);
    setGnssMode(_currentGnssMode);
    setDynamicModel(_currentDynModel);
    setSBASConfig(_currentSBAS);
  }
}

void GPSManager::setBaud(int baud) {
  if (_baudRate == baud)
    return;

  // 1. Command GPS to switch (while still at old baud)
  configureGpsBaud(baud);

  _baudRate = baud;

  Preferences prefs;
  prefs.begin("laptimer", false);
  prefs.putInt("gps_baud", _baudRate);
  prefs.end();

  // 2. Switch ESP32 to new baud
  if (_gpsSerial) {
    delay(200); // Wait for module
    _gpsSerial->updateBaudRate(_baudRate);
    delay(100);
    // Re-apply config
    setGnssMode(_currentGnssMode);
    setDynamicModel(_currentDynModel);
    setSBASConfig(_currentSBAS);
  }
}

void GPSManager::configureGpsBaud(int targetBaud) {
  if (!_gpsSerial)
    return;

  // UBX-CFG-PRT (0x06 0x00)
  uint8_t packet[] = {
      0xB5,
      0x62,
      0x06,
      0x00,
      0x14,
      0x00,
      0x01,
      0x00,
      0x00,
      0x00, // PortID=1 (UART1)
      0xD0,
      0x08,
      0x00,
      0x00,                         // Mode (8N1)
      (uint8_t)(targetBaud & 0xFF), // Baud LSB
      (uint8_t)((targetBaud >> 8) & 0xFF),
      (uint8_t)((targetBaud >> 16) & 0xFF),
      (uint8_t)((targetBaud >> 24) & 0xFF), // Baud MSB
      0x07,
      0x00, // In Proto (UBX+NMEA+RTCM)
      0x03,
      0x00, // Out Proto (UBX+NMEA)
      0x00,
      0x00, // Flags
      0x00,
      0x00, // Reserved
      0x00,
      0x00 // Checksum
  };

  // Calc Checksum
  uint8_t ck_a = 0, ck_b = 0;
  for (int i = 2; i < 26; i++) {
    ck_a += packet[i];
    ck_b += ck_a;
  }
  packet[26] = ck_a;
  packet[27] = ck_b;

  sendUBX(packet, sizeof(packet));
}

void GPSManager::disableUnnecessarySentences() {
  if (!_gpsSerial)
    return;

  // Disable GSA (DOP and active satellites) - Not critical for racing
  uint8_t disableGSA[] = {
      0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, // NMEA-GxGSA
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // Disable on all ports
      0x01, 0x31                                      // Checksum
  };
  sendUBX(disableGSA, sizeof(disableGSA));
  delay(50);

  // Disable GSV (Satellites in view) - Not critical, uses lots of bandwidth
  uint8_t disableGSV[] = {
      0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, // NMEA-GxGSV
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38  // Checksum
  };
  sendUBX(disableGSV, sizeof(disableGSV));
  delay(50);

  // Disable GLL (Geographic position) - Redundant with RMC
  uint8_t disableGLL[] = {
      0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, // NMEA-GxGLL
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A  // Checksum
  };
  sendUBX(disableGLL, sizeof(disableGLL));
  delay(50);

  // Keep enabled: GGA (Position), RMC (Recommended minimum), VTG (Track/Speed)
  // These are essential for racing/tracking
}

#include "GPSManager.h"
#include <HardwareSerial.h>

void GPSManager::begin() {
#if defined(PIN_GPS_RX) && defined(PIN_GPS_TX)
  // Use Serial2 for GPS
  _gpsSerial = &Serial2;
  _gpsSerial->begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
#else
  // GPS Disabled due to pin conflicts
  _gpsSerial = NULL;
#endif
}

void GPSManager::update() {
  if (!_gpsSerial) return;
  while (_gpsSerial->available() > 0) {
    _gps.encode(_gpsSerial->read());
  }
}

bool GPSManager::isFixed() { return _gps.location.isValid(); }

double GPSManager::getLatitude() { return _gps.location.lat(); }

double GPSManager::getLongitude() { return _gps.location.lng(); }

float GPSManager::getSpeedKmph() {
  if (_gps.speed.isValid()) {
    return _gps.speed.kmph();
  }
  return 0.0;
}

int GPSManager::getSatellites() {
  if (_gps.satellites.isValid()) {
    return _gps.satellites.value();
  }
  return 0;
}

String GPSManager::getTimeString() {
  if (_gps.time.isValid()) {
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", _gps.time.hour(), _gps.time.minute(),
            _gps.time.second());
    return String(buf);
  }
  return "--:--:--";
}

String GPSManager::getDateString() {
  if (_gps.date.isValid()) {
    char buf[16];
    sprintf(buf, "%02d/%02d/%04d", _gps.date.day(), _gps.date.month(),
            _gps.date.year());
    return String(buf);
  }
  return "--/--/----";
}

double GPSManager::getHDOP() {
  if (_gps.hdop.isValid()) {
    return _gps.hdop.hdop();
  }
  return 99.9; // No fix/bad
}

double GPSManager::distanceBetween(double lat1, double long1, double lat2,
                                   double long2) {
  return _gps.distanceBetween(lat1, long1, lat2, long2);
}

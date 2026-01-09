#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include "../config.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h> // Ensure SPI is included
#include <TinyGPS++.h>

class GPSManager {
public:
  enum GnssMode {
    MODE_ALL_10HZ = 0,
    MODE_GPS_GL_SBAS_16HZ = 1, // Default
    MODE_GPS_GAL_GL_SBAS_10HZ = 2,
    MODE_GPS_GAL_SBAS_20HZ = 3,
    MODE_GPS_SBAS_25HZ = 4
  };

  void begin();
  void update();

  // Getters
  bool isFixed();
  double getLatitude();
  double getLongitude();
  float getSpeedKmph();
  float getTotalTrip();
  void resetTrip();
  int getSatellites();
  void setRPM(int rpm) { _currentRPM = rpm; }
  int getRPM() { return _currentRPM; }
  String getTimeString();
  String getDateString();
  int getRawHour();
  int getRawMinute();
  double getHDOP();
  int getUpdateRate();

  // Configuration
  void setGnssMode(uint8_t mode);
  void setDynamicModel(uint8_t model); // 0=Portable, 1=Station, 2=Ped...
  void setSBASConfig(uint8_t region);  // 0=EGNOS, 1=WAAS...
  void setProjection(bool enabled);    // Coordinate Projection
  bool isProjectionEnabled() { return _projectionEnabled; }
  void setFrequencyLimit(int freq); // 10 or 25 (max)

  // Manual Time & Redundancy
  void setManualTime(int h, int m, int s = 0);
  void setUtcOffset(int offset); // Hours
  int getUtcOffset() { return _utcOffset; }

  // Pin Configuration
  void setPins(int rx, int tx);
  int getRxPin() { return _rxPin; }
  int getTxPin() { return _txPin; }

  // Baud Rate Config
  void setBaud(int baud);
  int getBaud() { return _baudRate; }

  // Debugging / Logging
  // Callback signature: void(uint8_t c)
  typedef std::function<void(uint8_t)> RawDataCallback;
  void setRawDataCallback(RawDataCallback cb) { _dataCallback = cb; }

  // Utilities
  double distanceBetween(double lat1, double long1, double lat2, double long2);
  void getLocalTime(int &h, int &m, int &s, int &d, int &mo, int &y);

private:
  TinyGPSPlus _gps;
  HardwareSerial *_gpsSerial;
  void sendUBX(const uint8_t *cmd, int len);

  RawDataCallback _dataCallback = nullptr;

  // Pin Config
  int _rxPin = PIN_GPS_RX; // Default from config.h
  int _txPin = PIN_GPS_TX;
  int _baudRate = GPS_BAUD; // Default 9600

  double _totalDistance = 0.0;
  double _lastLat = 0.0;
  double _lastLng = 0.0;
  bool _hasLastPos = false;
  unsigned long _lastSaveTime = 0;
  int _currentRPM = 0;

  // Hz Calculation
  int _updatesCount = 0;
  unsigned long _lastRateCheck = 0;
  int _currentHz = 0;

  // Settings Cache
  uint8_t _currentGnssMode = 0;
  uint8_t _currentDynModel = 3;   // Default Automotive (User Index 3 -> UBX 4)
  uint8_t _currentSBAS = 0;       // Default EGNOS
  bool _projectionEnabled = true; // Default Enabled
  int _targetFreq = 10;

  // Manual Time System
  int _sysHour = 0;
  int _sysMin = 0;
  int _sysSec = 0;
  int _sysDay = 1;
  int _sysMonth = 1;
  int _sysYear = 2024;
  int _utcOffset = 0;
  unsigned long _lastTick = 0;
};

#endif

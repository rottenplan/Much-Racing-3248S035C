#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include "../config.h"
#include <TinyGPS++.h>

class GPSManager {
public:
  void begin();
  void update();

  // Getters
  bool isFixed();
  double getLatitude();
  double getLongitude();
  float getSpeedKmph();
  int getSatellites();
  String getTimeString();
  String getDateString();
  double getHDOP();

  // Utilities
  double distanceBetween(double lat1, double long1, double lat2, double long2);

private:
  TinyGPSPlus _gps;
  HardwareSerial *_gpsSerial;
};

#endif

#ifndef TRACK_DATA_H
#define TRACK_DATA_H

#include <Arduino.h>
#include <vector>

// GPS Point for track recording
struct GPSPoint {
  double lat;
  double lon;
  unsigned long timestamp;
};

struct TrackConfig {
  String name;
  // TODO: Add sectors/finish line coordinates here
};

struct Track {
  String name;
  std::vector<TrackConfig> configs;
  double lat;
  double lon;
  bool isCustom = true;
  String pathFile; // Path to CSV file containing track points
  unsigned long bestLap = 0;
};

struct ActiveSessionData {
  String trackName;
  std::vector<unsigned long> lapTimes;
  unsigned long bestLapTime;
  int lapCount;
  int maxRpm;
  std::vector<GPSPoint> trackPoints;
};

#endif

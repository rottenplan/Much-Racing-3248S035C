#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../config.h"
#include <Arduino.h>
#include <FS.h>
#include <SD.h>

class SessionManager {
public:
  void begin();

  bool startSession();
  void stopSession();
  void logData(String dataLine); // Simplified logging

  bool isLogging() { return _logging; }

  void appendToHistoryIndex(String filename, String date, int laps,
                            unsigned long bestLap);
  String loadHistoryIndex(); // Returns full content for processing

  bool getSDStatus(uint64_t &total, uint64_t &used);

  struct SDTestResult {
    bool success;
    String cardType;
    String sizeLabel; // e.g. "16 GB"
    String usedLabel; // e.g. "200 MB"
    float readSpeedKBps;
    float writeSpeedKBps;
  };

  SDTestResult runFullTest();

private:
  bool _logging;
  File _logFile;
  String createFilename();
};

#endif

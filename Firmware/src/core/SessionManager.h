#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../config.h"
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class SessionManager {
public:
  void begin();

  bool startSession();
  void stopSession();
  void logData(String dataLine); // Simplified logging

  bool isLogging() { return _logging; }

  void appendToHistoryIndex(String filename, String date, int laps,
                            unsigned long bestLap, String type = "TRACK");
  String loadHistoryIndex(); // Returns full content for processing

  bool deleteSession(String filename); // Delete file and update index

  bool getSDStatus(uint64_t &total, uint64_t &used);

  struct SDTestResult {
    bool success;
    String cardType;
    String sizeLabel; // e.g. "16 GB"
    String usedLabel; // e.g. "200 MB"
    float readSpeedKBps;
    float writeSpeedKBps;
  };

  SDTestResult runFullTest(void (*progressCallback)(int, String) = NULL);

private:
  bool _logging;
  File _logFile;
  String createFilename();
  String _currentFilename;

public:
  String getCurrentFilename() { return _currentFilename; }

private:
  QueueHandle_t _logQueue;
  TaskHandle_t _loggingTaskHandle;
  static void loggingTask(void *parameter);
};

#endif

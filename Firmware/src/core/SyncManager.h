#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>

class SyncManager {
public:
  SyncManager();

  // Main sync functions
  bool performFirstSync(const char *apiUrl, const char *username,
                        const char *password);
  bool syncSettings(const char *apiUrl, const char *username,
                    const char *password);
  bool uploadSessions(const char *apiUrl, const char *username,
                      const char *password);
  bool uploadGPXTracks(const char *apiUrl, const char *username,
                       const char *password);

  // Status checks
  bool isFirstSyncDone();
  String getLastSyncTime();

  // Manual sync trigger
  void triggerManualSync();

private:
  Preferences _prefs;

  // HTTP helpers
  String makeBasicAuthHeader(const char *username, const char *password);
  bool downloadAndApplySettings(const char *apiUrl, const char *authHeader);
  bool downloadTracks(const char *apiUrl, const char *authHeader);

  // Settings application
  void applySettings(JsonDocument &doc);
  void applyTrackSelection(JsonDocument &doc);
  void applyEngineSettings(JsonDocument &doc);

  // Storage
  void markSyncComplete();
  void saveLastSyncTime();
};

#endif

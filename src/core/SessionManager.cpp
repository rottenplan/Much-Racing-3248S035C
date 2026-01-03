#include "SessionManager.h"
#include <SPI.h>

void SessionManager::begin() {
  _logging = false;

  Serial.printf("SD Init: SCK=%d, MISO=%d, MOSI=%d, CS=%d\n", PIN_SD_SCLK,
                PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  // Gunakan instans SPI khusus untuk Kartu SD (VSPI)
  // Ini menghindari konflik dengan Tampilan/Sentuh (biasanya pada HSPI)
  SPIClass *sdSpi = new SPIClass(VSPI);
  sdSpi->begin(PIN_SD_SCLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  delay(10); // Tunggu SPI stabil

  // Teruskan SPI khusus ke SD.begin
  if (!SD.begin(PIN_SD_CS, *sdSpi, 4000000)) { // Kecepatan aman 4MHz
    Serial.println("SD Card Init Failed!");
  } else {
    Serial.println("SD Card Ready");
    if (!SD.exists("/sessions")) {
      SD.mkdir("/sessions");
    }
  }
}

bool SessionManager::startSession() {
  if (_logging)
    return true;

  String filename = createFilename();
  _logFile = SD.open(filename, FILE_WRITE);

  if (_logFile) {
    _logging = true;
    _logFile.println("Time,Lat,Lon,Speed,Sats"); // Header
    Serial.println("Started logging to: " + filename);
    return true;
  }
  return false;
}

void SessionManager::stopSession() {
  if (_logging && _logFile) {
    _logFile.close();
    _logging = false;
    Serial.println("Session Stopped");
  }
}

void SessionManager::logData(String dataLine) {
  if (_logging && _logFile) {
    _logFile.println(dataLine);
  }
}

String SessionManager::createFilename() {
  int i = 0;
  String fn;
  do {
    fn = "/sessions/run_" + String(i) + ".csv";
    i++;
  } while (SD.exists(fn));
  return fn;
}

void SessionManager::appendToHistoryIndex(String filename, String date,
                                          int laps, unsigned long bestLap) {
  File indexFile = SD.open("/history.csv", FILE_APPEND);
  if (!indexFile) {
    indexFile = SD.open("/history.csv", FILE_WRITE);
  }

  if (indexFile) {
    // Format: NamaFile,Tanggal,Lap,LapTerbaik
    String line =
        filename + "," + date + "," + String(laps) + "," + String(bestLap);
    indexFile.println(line);
    indexFile.close();
    Serial.println("Added to history index: " + line);
  } else {
    Serial.println("Failed to open history index");
  }
}

String SessionManager::loadHistoryIndex() {
  if (!SD.exists("/history.csv"))
    return "";

  File indexFile = SD.open("/history.csv", FILE_READ);
  if (!indexFile)
    return "";

  String content = "";
  while (indexFile.available()) {
    content += (char)indexFile.read();
  }
  indexFile.close();
  indexFile.close();
  return content;
}

bool SessionManager::getSDStatus(uint64_t &total, uint64_t &used) {
  if (!SD.totalBytes())
    return false; // Periksa apakah terpasang/valid
  total = SD.totalBytes();
  used = SD.usedBytes();
  return true;
}

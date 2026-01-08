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
    _currentFilename = filename;
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
                                          int laps, unsigned long bestLap,
                                          String type) {
  File indexFile = SD.open("/history.csv", FILE_APPEND);
  if (!indexFile) {
    indexFile = SD.open("/history.csv", FILE_WRITE);
  }

  if (indexFile) {
    // Format: NamaFile,Tanggal,Lap,LapTerbaik,Tipe
    String line = filename + "," + date + "," + String(laps) + "," +
                  String(bestLap) + "," + type;
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
  indexFile.close();
  indexFile.close();
  return content;
}

bool SessionManager::deleteSession(String filename) {
  // 1. Remove the actual log file
  if (SD.exists(filename)) {
    SD.remove(filename);
    Serial.println("Deleted log file: " + filename);
  } else {
    Serial.println("Log file not found: " + filename);
    // Proceed to clean index anyway
  }

  // 2. Rewrite History Index
  if (!SD.exists("/history.csv"))
    return false;

  File inFile = SD.open("/history.csv", FILE_READ);
  if (!inFile)
    return false;

  String tempPath = "/history.tmp";
  File outFile = SD.open(tempPath, FILE_WRITE);
  if (!outFile) {
    inFile.close();
    return false;
  }

  bool found = false;
  while (inFile.available()) {
    String line = inFile.readStringUntil('\n');
    line.trim();
    if (line.length() == 0)
      continue;

    // Check if this line contains the filename
    // Format: /sessions/run_X.csv,Date,...
    int c1 = line.indexOf(',');
    if (c1 > 0) {
      String fName = line.substring(0, c1);
      if (fName == filename) {
        found = true;
        Serial.println("Skipping deleted entry in index: " + fName);
        continue; // Skip this line
      }
    }
    outFile.println(line);
  }

  inFile.close();
  outFile.close();

  SD.remove("/history.csv");
  SD.rename(tempPath, "/history.csv");

  return true;
}

bool SessionManager::getSDStatus(uint64_t &total, uint64_t &used) {
  if (!SD.totalBytes())
    return false; // Periksa apakah terpasang/valid
  total = SD.totalBytes();
  used = SD.usedBytes();
  return true;
}

SessionManager::SDTestResult
SessionManager::runFullTest(void (*progressCallback)(int, String)) {
  SDTestResult res;
  res.success = false;
  res.readSpeedKBps = 0;
  res.writeSpeedKBps = 0;

  if (!SD.totalBytes()) {
    res.cardType = "NO CARD";
    return res;
  }

  uint64_t total = SD.totalBytes();
  uint64_t used = SD.usedBytes();

  // Format Size Label
  float totalGB = total / (1024.0 * 1024.0 * 1024.0);
  res.sizeLabel = String(totalGB, 1) + " GB";

  float usedMB = used / (1024.0 * 1024.0);
  res.usedLabel = String(usedMB, 0) + " MB";

  sdcard_type_t t = SD.cardType();
  if (t == CARD_MMC)
    res.cardType = "MMC";
  else if (t == CARD_SD)
    res.cardType = "SDSC";
  else if (t == CARD_SDHC)
    res.cardType = "SDHC";
  else
    res.cardType = "UNKNOWN";

  // Benchmark Write
  if (progressCallback)
    progressCallback(0, "Writing...");

  uint8_t buf[512]; // Small buffer
  memset(buf, 0xAA, 512);
  String testFile = "/test_bench.bin";

  unsigned long start = millis();
  File f = SD.open(testFile, FILE_WRITE);
  if (f) {
    // Write 1MB (2048 * 512)
    int chunks = 2048;
    for (int i = 0; i < chunks; i++) {
      f.write(buf, 512);
      if (i % 200 == 0 && progressCallback) { // Update every ~10% of phase
        int p = (i * 50) / chunks;            // 0-50%
        progressCallback(p, "Writing...");
      }
    }
    f.close();
    unsigned long duration = millis() - start;
    if (duration > 0) {
      res.writeSpeedKBps = 1024.0 / (duration / 1000.0);
    }
  } else {
    return res; // Write failed
  }

  // Benchmark Read
  if (progressCallback)
    progressCallback(50, "Reading...");

  start = millis();
  f = SD.open(testFile, FILE_READ);
  if (f) {
    long len = f.size();
    long pos = 0;
    int chunks = 0;

    while (f.available()) {
      f.read(buf, 512);
      pos += 512;
      chunks++;

      if (chunks % 200 == 0 && progressCallback) {
        int p = 50 + (pos * 50) / len; // 50-100%
        progressCallback(p, "Reading...");
      }
    }
    f.close();
    unsigned long duration = millis() - start;
    if (duration > 0) {
      res.readSpeedKBps = 1024.0 / (duration / 1000.0);
    }
  } else {
    return res; // Read failed
  }

  // Cleanup
  if (progressCallback)
    progressCallback(100, "Done!");
  SD.remove(testFile);

  res.success = true;
  return res;
}

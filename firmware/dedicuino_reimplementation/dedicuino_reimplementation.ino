#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "dedicuino_logo.h"

#if __has_include("private_brand_logo.h")
#include "private_brand_logo.h"
#define DEDICUINO_HAS_PRIVATE_BRAND_LOGO 1
#else
#define DEDICUINO_HAS_PRIVATE_BRAND_LOGO 0
#endif

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include "wifi_secrets.h"
#include "web_dashboard.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
WebServer webServer(80);

#ifndef WIFI_OTA_HOSTNAME
#define WIFI_OTA_HOSTNAME "dedicuino"
#endif

#ifndef WIFI_OTA_PASSWORD
#define WIFI_OTA_PASSWORD ""
#endif

static bool otaInProgress = false;
static uint8_t otaProgressPercent = 0;
#endif

// =============================
// Dedicuino Reimplementation
// =============================
// Features implemented (from README):
// - Real-time shot timer based on pressure threshold
// - Pressure: current, max, and post-shot average/max
// - Temperature: current, max, and post-shot average/max
// - OLED live visualization (128x64 I2C)
// - Standalone monitoring only (non-invasive to machine electronics)

// ---------- OLED ----------
static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = 0;
static const uint8_t OLED_I2C_ADDR = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Board ----------
// Target board: Arduino Nano ESP32

// ---------- Pins ----------
static const uint8_t PIN_PRESSURE = A0;   // Pressure sensor signal
static const uint8_t PIN_TEMP_THERMISTOR = A1; // Thermistor divider signal

#if defined(ARDUINO_ARCH_ESP32)
static const float ADC_MAX_COUNTS = 4095.0f;
#else
static const float ADC_MAX_COUNTS = 1023.0f;
#endif

// ---------- Timing ----------
static const uint32_t SAMPLE_INTERVAL_MS = 100;
static const uint32_t EXTRACT_STOP_HOLD_MS = 300;
static const uint32_t POST_SHOT_DISPLAY_MS = 60000;

// ---------- Pressure calibration ----------
const int ADC_resolution = 12;
const float raw_max = pow(2, ADC_resolution) - 1;
static const float PRESSURE_PSI_MAX = 300.0f;
static const float PSI_TO_BAR = 0.0689476f;
static const float PRESSURE_CAL_FACTOR = 1.22f; // Adjust based on calibration against known pressure values

// ---------- Thermistor calibration (ATC Semitec 104GT-2) ----------
static const float THERMISTOR_NOMINAL_OHMS = 100000.0f; // 100k at 25°C
static const float THERMISTOR_NOMINAL_C = 25.0f;
static const float THERMISTOR_BETA = 4267.0f;
static const float THERMISTOR_SERIES_OHMS = 100000.0f; // fixed resistor in divider

// Shot detection thresholds (bar)
static const float SHOT_START_BAR = 1.0f;
static const float SHOT_STOP_BAR = 0.8f;

// ---------- Filtering ----------
static const float EMA_ALPHA = 0.25f; // Exponential moving average

enum BrewState {
  STATE_IDLE,
  STATE_EXTRACTING,
  STATE_POST_SHOT
};

struct ShotStats {
  float pMaxBar = 0.0f;
  float tMaxC = 0.0f;
  float pSumBar = 0.0f;
  float tSumC = 0.0f;
  uint32_t sampleCount = 0;
  uint32_t durationMs = 0;
};

BrewState state = STATE_IDLE;
ShotStats currentShot;
ShotStats lastShot;

float pressureBar = 0.0f;
float tempC = 0.0f;
float pressureBarFiltered = 0.0f;
float tempCFiltered = 25.0f;

uint32_t lastSampleMs = 0;
uint32_t shotStartMs = 0;
uint32_t belowStopSinceMs = 0;
uint32_t postShotSinceMs = 0;

float clampf(float x, float lo, float hi) {
  return (x < lo) ? lo : ((x > hi) ? hi : x);
}

float readPressureBar() {
  int raw = analogRead(PIN_PRESSURE);
  int offset = 343;
  float norm = ((raw - offset)/(raw_max - offset));
  // norm = clampf(norm, 0.0f, 1.0f);
  float psi = norm * PRESSURE_PSI_MAX*PRESSURE_CAL_FACTOR;
  return psi * PSI_TO_BAR;
}

float readAnalogTempC() {
  int raw = analogRead(PIN_TEMP_THERMISTOR);

  // Protect against division by zero at rail values.
  if (raw <= 0 || raw >= (int)ADC_MAX_COUNTS) {
    return tempCFiltered;
  }

  // Divider: Thermistor to 3.3V, fixed resistor to GND, node to ADC.
  const float ratio = raw / ADC_MAX_COUNTS;
  const float rTherm = THERMISTOR_SERIES_OHMS * ((1.0f / ratio) - 1.0f);
  if (rTherm <= 0.0f) {
    return tempCFiltered;
  }

  // Beta model
  const float t0K = THERMISTOR_NOMINAL_C + 273.15f;
  const float invTK = (1.0f / t0K) + (1.0f / THERMISTOR_BETA) * log(rTherm / THERMISTOR_NOMINAL_OHMS);
  const float tC = (1.0f / invTK) - 273.15f;

  // Keep last filtered value if sensor is disconnected/invalid.
  if (tC < -40.0f || tC > 170.0f) {
    return tempCFiltered;
  }
  return tC;
}

void resetCurrentShot() {
  currentShot = ShotStats();
}

void beginShot(uint32_t now) {
  resetCurrentShot();
  shotStartMs = now;
  belowStopSinceMs = 0;
  state = STATE_EXTRACTING;
}

void finalizeShot(uint32_t now) {
  currentShot.durationMs = now - shotStartMs;
  lastShot = currentShot;
  postShotSinceMs = now;
  state = STATE_POST_SHOT;
}

void updateShotStats() {
  currentShot.sampleCount++;
  currentShot.pSumBar += pressureBarFiltered;
  currentShot.tSumC += tempCFiltered;

  if (pressureBarFiltered > currentShot.pMaxBar) currentShot.pMaxBar = pressureBarFiltered;
  if (tempCFiltered > currentShot.tMaxC) currentShot.tMaxC = tempCFiltered;
}

float shotAvgPressure(const ShotStats &s) {
  if (s.sampleCount == 0) return 0.0f;
  return s.pSumBar / s.sampleCount;
}

float shotAvgTemp(const ShotStats &s) {
  if (s.sampleCount == 0) return 0.0f;
  return s.tSumC / s.sampleCount;
}

#if defined(ARDUINO_ARCH_ESP32)
const char* brewStateToString(BrewState s) {
  switch (s) {
    case STATE_IDLE: return "IDLE";
    case STATE_EXTRACTING: return "EXTRACTING";
    case STATE_POST_SHOT: return "POST_SHOT";
    default: return "UNKNOWN";
  }
}

float getShotFillPercent(uint32_t now) {
  if (state == STATE_EXTRACTING) {
    float elapsedMs = (float)(now - shotStartMs);
    return clampf((elapsedMs / 30000.0f) * 100.0f, 8.0f, 100.0f);
  }
  if (state == STATE_POST_SHOT) {
    return 100.0f;
  }
  return 0.0f;
}

void setupWebInterface() {
  webServer.on("/", HTTP_GET, []() {
    webServer.send_P(200, "text/html", WEB_DASHBOARD_HTML);
  });

  webServer.on("/api/status", HTTP_GET, []() {
    uint32_t now = millis();

    float timerSeconds = 0.0f;
    float pAvg = 0.0f;
    float tAvg = 0.0f;
    float pMax = 0.0f;
    float tMax = 0.0f;

    if (state == STATE_EXTRACTING) {
      timerSeconds = (now - shotStartMs) / 1000.0f;
      pAvg = shotAvgPressure(currentShot);
      tAvg = shotAvgTemp(currentShot);
      pMax = currentShot.pMaxBar;
      tMax = currentShot.tMaxC;
    } else if (state == STATE_POST_SHOT) {
      timerSeconds = lastShot.durationMs / 1000.0f;
      pAvg = shotAvgPressure(lastShot);
      tAvg = shotAvgTemp(lastShot);
      pMax = lastShot.pMaxBar;
      tMax = lastShot.tMaxC;
    }

    String json = "{";
    json += "\"state\":\"" + String(brewStateToString(state)) + "\",";
    json += "\"timerSeconds\":" + String(timerSeconds, 1) + ",";
    json += "\"pressureBar\":" + String(pressureBarFiltered, 1) + ",";
    json += "\"tempC\":" + String(tempCFiltered, 1) + ",";
    json += "\"shotFillPct\":" + String(getShotFillPercent(now), 1) + ",";
    json += "\"pAvg\":" + String(pAvg, 1) + ",";
    json += "\"tAvg\":" + String(tAvg, 1) + ",";
    json += "\"pMax\":" + String(pMax, 1) + ",";
    json += "\"tMax\":" + String(tMax, 1) + ",";
    json += "\"wifiRssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"uptimeSec\":" + String(now / 1000.0f, 1);
    json += "}";

    webServer.send(200, "application/json", json);
  });

  webServer.onNotFound([]() {
    webServer.send(404, "text/plain", "Not Found");
  });

  webServer.begin();
}
#endif
void drawTimerIcon(int16_t x, int16_t y) {
  // 24x24 stopwatch icon
  display.drawCircle(x + 12, y + 13, 10, SSD1306_WHITE);
  display.drawRect(x + 9, y + 0, 6, 3, SSD1306_WHITE);   // crown
  display.drawLine(x + 16, y + 4, x + 19, y + 2, SSD1306_WHITE); // side button
  display.drawLine(x + 12, y + 13, x + 12, y + 7, SSD1306_WHITE); // minute hand
  display.drawLine(x + 12, y + 13, x + 17, y + 15, SSD1306_WHITE); // second hand
}

void drawThermometerIcon(int16_t x, int16_t y) {
  // 12x28 simple thermometer icon
  display.drawRoundRect(x + 4, y + 0, 4, 20, 2, SSD1306_WHITE);
  display.fillRect(x + 5, y + 10, 2, 9, SSD1306_WHITE);
  display.drawCircle(x + 6, y + 23, 5, SSD1306_WHITE);
  display.fillCircle(x + 6, y + 23, 3, SSD1306_WHITE);
}

void drawPressureIcon(int16_t x, int16_t y) {
  // 12x28 simple gauge icon
  display.drawCircle(x + 6, y + 10, 6, SSD1306_WHITE);
  display.drawLine(x + 6, y + 10, x + 10, y + 7, SSD1306_WHITE);
  display.drawLine(x + 3, y + 20, x + 9, y + 20, SSD1306_WHITE);
  display.drawLine(x + 2, y + 22, x + 10, y + 22, SSD1306_WHITE);
  display.drawLine(x + 1, y + 24, x + 11, y + 24, SSD1306_WHITE);
}

void drawCupAnimation(int16_t x, int16_t y, uint32_t now) {
  // Compact cup icon (14x18) for the top-right timer area.
  // States:
  // - IDLE: empty cup
  // - EXTRACTING: drops + filling cup
  // - POST_SHOT: full cup

  // Cup body and rim
  display.drawRoundRect(x, y + 3, 10, 12, 2, SSD1306_WHITE);
  display.drawLine(x + 1, y + 3, x + 8, y + 3, SSD1306_WHITE);

  // Cup handle
  display.drawPixel(x + 10, y + 6, SSD1306_WHITE);
  display.drawPixel(x + 11, y + 7, SSD1306_WHITE);
  display.drawPixel(x + 11, y + 8, SSD1306_WHITE);
  display.drawPixel(x + 10, y + 9, SSD1306_WHITE);

  // Saucer
  display.drawLine(x - 1, y + 16, x + 11, y + 16, SSD1306_WHITE);

  uint8_t fillPercent = 0;
  bool showDrops = false;

  if (state == STATE_EXTRACTING) {
    showDrops = true;
    fillPercent = (uint8_t)getShotFillPercent(now);
  } else if (state == STATE_POST_SHOT) {
    fillPercent = 100;
  }

  // Fill level inside cup
  if (fillPercent > 0) {
    const int16_t innerX = x + 1;
    const int16_t innerY = y + 5;
    const int16_t innerW = 8;
    const int16_t innerH = 9;
    int16_t fillH = (int16_t)((innerH * fillPercent) / 100);
    if (fillH > innerH) fillH = innerH;
    display.fillRect(innerX, innerY + (innerH - fillH), innerW, fillH, SSD1306_WHITE);
  }

  // Animated coffee drops while extracting
  if (showDrops) {
    uint8_t frame = (uint8_t)((now / 120U) % 4U);
    int16_t dropX = x + 4;

    if (frame == 0) {
      display.drawPixel(dropX, y + 0, SSD1306_WHITE);
    } else if (frame == 1) {
      display.drawPixel(dropX, y + 1, SSD1306_WHITE);
      display.drawPixel(dropX + 2, y + 0, SSD1306_WHITE);
    } else if (frame == 2) {
      display.drawPixel(dropX, y + 2, SSD1306_WHITE);
      display.drawPixel(dropX + 2, y + 1, SSD1306_WHITE);
    } else {
      display.drawPixel(dropX, y + 3, SSD1306_WHITE);
      display.drawPixel(dropX + 2, y + 2, SSD1306_WHITE);
    }
  }
}

void drawAvgIcon(int16_t x, int16_t y) {
  // requested 8x8 bitmap:
  // -------#
  // ---##-#-
  // --#--#--
  // -#--#-#-
  // -#-#--#-
  // --#--#--
  // -#-##---
  // #-------
  static const uint8_t AVG_ICON_8X8[] PROGMEM = {
    0x01, 0x1A, 0x24, 0x4A, 0x52, 0x24, 0x58, 0x80
  };
  display.drawBitmap(x, y, AVG_ICON_8X8, 8, 8, SSD1306_WHITE);
}
void drawMaxIcon(int16_t x, int16_t y) {
  // requested 8x8 bitmap:
  // ---#----
  // --###---
  // -#-#-#--
  // #--#--#-
  // ---#----
  // ---#----
  // ---#----
  // ---#----
  static const uint8_t MAX_ICON_8X8[] PROGMEM = {
  0x00, 0x10, 0x38, 0x38, 0x54, 0x54, 0x10, 0x10};
  display.drawBitmap(x, y, MAX_ICON_8X8, 8, 8, SSD1306_WHITE);
}

void drawValueOrDashes(float value, bool showValue) {
  if (showValue) {
    display.print(value, 1);
  } else {
    display.print("---");
  }
}
void drawTempCompact4(float value) {
  if (value <= 100.0f && value >= 10.0f) {
    display.print("0");
  }
  if (value < 10.0f && value >= 0.0f) {
    display.print("00");
  }
  display.print((int)roundf(value));
}

#if defined(ARDUINO_ARCH_ESP32)
void drawWifiIcon(int16_t x, int16_t y) {
  // 9x7 WiFi icon:
  // -#######-
  // #-------#
  // --#####--
  // -#-----#-
  // ---###---
  // --#---#--
  // ----#----
  static const char* WIFI_ICON[7] = {
    "-#######-",
    "#-------#",
    "--#####--",
    "-#-----#-",
    "---###---",
    "--#---#--",
    "----#----"
  };

  for (int row = 0; row < 7; ++row) {
    for (int col = 0; col < 9; ++col) {
      if (WIFI_ICON[row][col] == '#') {
        display.drawPixel(x + col, y + row, SSD1306_WHITE);
      }
    }
  }

}

void drawConnectivityStatus() {
  const bool wifiConnected = (WiFi.status() == WL_CONNECTED);

  // Top-right compact icon status (WiFi)
  if (wifiConnected) {
    drawWifiIcon(108, 0);
  }
}
#endif

void drawMainLayout(uint32_t now) {
  // Determine values by state
  float timerSeconds = 0.0f;
  float pAvg = 0.0f;
  float tAvg = 0.0f;
  float pMax = 0.0f;
  float tMax = 0.0f;
  bool showStats = false;

  if (state == STATE_EXTRACTING) {
    timerSeconds = (now - shotStartMs) / 1000.0f;
    pAvg = shotAvgPressure(currentShot);
    tAvg = shotAvgTemp(currentShot);
    pMax = currentShot.pMaxBar;
    tMax = currentShot.tMaxC;
    showStats = currentShot.sampleCount > 0;
  } else if (state == STATE_POST_SHOT) {
    timerSeconds = lastShot.durationMs / 1000.0f;
    pAvg = shotAvgPressure(lastShot);
    tAvg = shotAvgTemp(lastShot);
    pMax = lastShot.pMaxBar;
    tMax = lastShot.tMaxC;
    showStats = lastShot.sampleCount > 0;
  }

  // Top row: timer icon + SS.d
  drawTimerIcon(0, 0);
  uint16_t sec = (uint16_t)timerSeconds;
  uint8_t dsec = (uint8_t)(timerSeconds * 10.0f) % 10;

  display.setTextSize(3);
  display.setCursor(36, 0);
  if (sec < 10) display.print('0');
  display.print(sec);

  display.setTextSize(1);
  display.setCursor(70, 0);
  display.print(".");
  display.print(dsec);

  // Top-right: cup animation (empty / filling with drops / full)
  drawCupAnimation(94, 6, now);
  
  display.drawLine(0, 28, 127, 28, SSD1306_WHITE);

  // Bottom left: temperature block
  drawThermometerIcon(0, 32);
  display.setTextSize(2);
  display.setCursor(16, 32);
  drawTempCompact4(tempCFiltered);
  display.setTextSize(1);
  drawAvgIcon(16,49);
  display.setCursor(26, 48);
  drawValueOrDashes(tAvg, showStats);
  drawMaxIcon(16, 56);
  display.setCursor(26, 56);
  drawValueOrDashes(tMax, showStats);

  // Bottom right: pressure block
  drawPressureIcon(64, 32);
  display.setTextSize(2);
  display.setCursor(80, 32);
  display.print(pressureBarFiltered, 1);
  display.setTextSize(1);
  drawAvgIcon(80, 48);
  display.setCursor(90, 48);
  drawValueOrDashes(pAvg, showStats);
  drawMaxIcon(80, 56);
  display.setCursor(90, 56);
  drawValueOrDashes(pMax, showStats);

//   // Tiny state marker
//   display.setCursor(80, 0);
//   if (state == STATE_IDLE) {
//     display.print("IDL");
//   } else if (state == STATE_EXTRACTING) {
//     display.print("RUN");
//   } else {
//     display.print("SUM");
//   }

#if defined(ARDUINO_ARCH_ESP32)
  drawConnectivityStatus();
#endif
}

void updateDisplay(uint32_t now) {
  display.clearDisplay();
  drawMainLayout(now);
  display.display();
}

#if defined(ARDUINO_ARCH_ESP32)
void drawOTAScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("OTA");

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Updating firmware...");

  const int16_t barX = 0;
  const int16_t barY = 38;
  const int16_t barW = 128;
  const int16_t barH = 10;
  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);

  int16_t fillW = (int16_t)((barW - 2) * (otaProgressPercent / 100.0f));
  if (fillW > 0) {
    display.fillRect(barX + 1, barY + 1, fillW, barH - 2, SSD1306_WHITE);
  }

  display.setCursor(0, 54);
  display.print("Progress: ");
  display.print(otaProgressPercent);
  display.print('%');
  display.display();
}

void setupOTA() {
  ArduinoOTA.setHostname(WIFI_OTA_HOSTNAME);

  if (strlen(WIFI_OTA_PASSWORD) > 0) {
    ArduinoOTA.setPassword(WIFI_OTA_PASSWORD);
  }

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    otaProgressPercent = 0;
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    otaProgressPercent = (uint8_t)((progress * 100U) / total);
  });

  ArduinoOTA.onEnd([]() {
    otaProgressPercent = 100;
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    otaInProgress = false;
  });

  ArduinoOTA.begin();
}
#endif

void showSplashBitmap(const uint8_t* bitmap, uint8_t width, uint8_t height, uint16_t holdMs) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const int16_t logoX = (SCREEN_WIDTH - width) / 2;
  const int16_t logoY = (SCREEN_HEIGHT - height) / 2;
  display.drawBitmap(logoX, logoY, bitmap, width, height, SSD1306_WHITE);
  display.display();
  delay(holdMs);
}

void setup() {
  pinMode(PIN_PRESSURE, INPUT);
  pinMode(PIN_TEMP_THERMISTOR, INPUT);

#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(12);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(ssid, password);
  setupOTA();
  setupWebInterface();
#endif

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    // OLED init failed; halt so failure is obvious.
    while (true) {
      delay(1000);
    }
  }

  showSplashBitmap(DE_LONGHI_LOGO_BITMAP, DE_LONGHI_LOGO_WIDTH, DE_LONGHI_LOGO_HEIGHT, 1300);
  showSplashBitmap(DEDICUINO_SPLASH_LOGO_BITMAP, DEDICUINO_SPLASH_LOGO_WIDTH, DEDICUINO_SPLASH_LOGO_HEIGHT, 1300);
#if DEDICUINO_HAS_PRIVATE_BRAND_LOGO
  showSplashBitmap(PRIVATE_BRAND_LOGO_BITMAP, PRIVATE_BRAND_LOGO_WIDTH, PRIVATE_BRAND_LOGO_HEIGHT, 1300);
#endif

  // Prime filters with first readings to avoid startup jumps.
  pressureBar = readPressureBar();
  tempC = readAnalogTempC();
  pressureBarFiltered = pressureBar;
  tempCFiltered = tempC;
}

void loop() {
#if defined(ARDUINO_ARCH_ESP32)
  ArduinoOTA.handle();
  webServer.handleClient();
  if (otaInProgress) {
    drawOTAScreen();
    return;
  }
#endif

  uint32_t now = millis();
  if (now - lastSampleMs < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleMs = now;

  pressureBar = readPressureBar();
  tempC = readAnalogTempC();

  pressureBarFiltered = (EMA_ALPHA * pressureBar) + ((1.0f - EMA_ALPHA) * pressureBarFiltered);
  tempCFiltered = (EMA_ALPHA * tempC) + ((1.0f - EMA_ALPHA) * tempCFiltered);

  switch (state) {
    case STATE_IDLE:
      if (pressureBarFiltered >= SHOT_START_BAR) {
        beginShot(now);
        updateShotStats();
      }
      break;

    case STATE_EXTRACTING:
      updateShotStats();

      if (pressureBarFiltered <= SHOT_STOP_BAR) {
        if (belowStopSinceMs == 0) {
          belowStopSinceMs = now;
        } else if (now - belowStopSinceMs >= EXTRACT_STOP_HOLD_MS) {
          finalizeShot(now);
        }
      } else {
        belowStopSinceMs = 0;
      }
      break;

    case STATE_POST_SHOT:
      // New shot can start immediately if pressure rises again.
      if (pressureBarFiltered >= SHOT_START_BAR) {
        beginShot(now);
        updateShotStats();
      } else if (now - postShotSinceMs >= POST_SHOT_DISPLAY_MS) {
        state = STATE_IDLE;
      }
      break;
  }

  updateDisplay(now);
}

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED ----------
static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = 0;
static const uint8_t OLED_I2C_ADDR = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Pin ----------
static const uint8_t PIN_PRESSURE = A0;

#if defined(ARDUINO_ARCH_ESP32)
static const float ADC_MAX_COUNTS = 4095.0f;
#else
static const float ADC_MAX_COUNTS = 1023.0f;
#endif

// ---------- Pressure calibration targets ----------
// Tune these two values while comparing with a trusted reference gauge.
static float PRESSURE_CAL_FACTOR = 1.22f;
static int PRESSURE_OFFSET = 343;

static const float PRESSURE_PSI_MAX = 300.0f;
static const float PSI_TO_BAR = 0.0689476f;

float clampf(float x, float lo, float hi) {
  return (x < lo) ? lo : ((x > hi) ? hi : x);
}

void setup() {
  pinMode(PIN_PRESSURE, INPUT);

#if defined(ARDUINO_ARCH_ESP32)
  analogReadResolution(12);
#endif

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

void loop() {
  const int raw = analogRead(PIN_PRESSURE);

  const float norm = clampf((raw - PRESSURE_OFFSET) / (ADC_MAX_COUNTS - PRESSURE_OFFSET), 0.0f, 1.0f);
  const float psi = norm * PRESSURE_PSI_MAX * PRESSURE_CAL_FACTOR;
  const float bar = psi * PSI_TO_BAR;

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RAW");
  display.setCursor(44, 0);
  display.print(raw);

  display.setCursor(0, 22);
  display.print("PSI");
  display.setCursor(44, 22);
  display.print(psi, 1);

  display.setCursor(0, 44);
  display.print("BAR");
  display.setCursor(44, 44);
  display.print(bar, 2);

  display.display();
  delay(120);
}

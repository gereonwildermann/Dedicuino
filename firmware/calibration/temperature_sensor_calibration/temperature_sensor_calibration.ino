#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// ---------- OLED ----------
static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = 0;
static const uint8_t OLED_I2C_ADDR = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Pin ----------
static const uint8_t PIN_TEMP_THERMISTOR = A1;

#if defined(ARDUINO_ARCH_ESP32)
static const float ADC_MAX_COUNTS = 4095.0f;
#else
static const float ADC_MAX_COUNTS = 1023.0f;
#endif

// ---------- Thermistor model (ATC Semitec 104GT-2 defaults) ----------
// Adjust these while validating against a trusted thermometer.
static float THERMISTOR_NOMINAL_OHMS = 100000.0f; // R0 at 25°C
static float THERMISTOR_NOMINAL_C = 25.0f;
static float THERMISTOR_BETA = 4267.0f;
static float THERMISTOR_SERIES_OHMS = 100000.0f;

void setup() {
  pinMode(PIN_TEMP_THERMISTOR, INPUT);

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
  const int raw = analogRead(PIN_TEMP_THERMISTOR);

  float tempC = NAN;
  float rTherm = NAN;

  if (raw > 0 && raw < (int)ADC_MAX_COUNTS) {
    const float ratio = raw / ADC_MAX_COUNTS;
    rTherm = THERMISTOR_SERIES_OHMS * ((1.0f / ratio) - 1.0f);

    if (rTherm > 0.0f) {
      const float t0K = THERMISTOR_NOMINAL_C + 273.15f;
      const float invTK = (1.0f / t0K) + (1.0f / THERMISTOR_BETA) * log(rTherm / THERMISTOR_NOMINAL_OHMS);
      tempC = (1.0f / invTK) - 273.15f;
    }
  }

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("RAW");
  display.setCursor(44, 0);
  display.print(raw);

  display.setCursor(0, 22);
  display.print("OHM");
  display.setCursor(44, 22);
  if (isnan(rTherm)) {
    display.print("---");
  } else {
    display.print((int)rTherm);
  }

  display.setCursor(0, 44);
  display.print("TEMP C");
  display.setCursor(44, 44);
  if (isnan(tempC)) {
    display.print("---");
  } else {
    display.print(tempC, 2);
  }

  display.display();
  delay(120);
}

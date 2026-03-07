# Calibration

This section contains standalone calibration sketches for Arduino Nano ESP32.

## Sketches

- `pressure_sensor_calibration/pressure_sensor_calibration.ino`
  - OLED shows only: `RAW`, `PSI`, `BAR`
  - Goal: tune `PRESSURE_OFFSET` and `PRESSURE_CAL_FACTOR`

- `temperature_sensor_calibration/temperature_sensor_calibration.ino`
  - OLED shows only: `RAW`, `OHM`, `TEMP C`
  - Goal: tune thermistor model constants (`THERMISTOR_BETA`, `THERMISTOR_NOMINAL_OHMS`, `THERMISTOR_SERIES_OHMS`)

## Notes

- Both sketches use the same OLED wiring (`0x3C`) as the main firmware.
- Pressure pin: `A0`
- Thermistor pin: `A1`
- On ESP32 builds, ADC is configured to 12-bit resolution.

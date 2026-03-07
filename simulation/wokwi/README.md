# Wokwi Real `.ino` Simulation

This project runs the **actual firmware sketch** from:

- [firmware/dedicuino_reimplementation/dedicuino_reimplementation.ino](../../firmware/dedicuino_reimplementation/dedicuino_reimplementation.ino)

The `sketch.ino` here is just an include wrapper.

## What is simulated

- Arduino Nano (Wokwi-compatible)
- SSD1306 OLED (I2C `0x3C`)
- Pressure input on `A0` (potentiometer `potPressure`)
- Analog thermistor input on `A1` (potentiometer `potTemp` emulates divider node)

## Run

1. Open this folder in Wokwi (web or VS Code extension).
2. Start simulation.
3. Adjust:
   - `potPressure` to cross shot thresholds (`1.0 bar` start, `0.8 bar` stop)
   - `potTemp` to emulate thermistor temperature changes

## Splash sequence now in firmware

The firmware startup splash runs in this order:

1. DeLonghi logo
2. Dedicuino logo
3. Optional private brand logo (only if local `private_brand_logo.h` exists)

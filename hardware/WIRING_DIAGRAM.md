# Dedicuino Reimplementation — Hardware Wiring Diagram

> **Note**: This is a reimplementation of the [original DeLonghi Dedica modification by CaiJonas](https://github.com/CaiJonas/DeLonghi-Dedica-EC885-EC685-modification/), adapted for Arduino Nano ESP32 with 3.3V sensor architecture and USB-C power.

This wiring uses an Arduino Nano ESP32, I2C OLED, analog pressure sensor, and an analog NTC thermistor (ATC Semitec 104GT-2).

## Pin Mapping

| Component | Signal | Arduino Nano ESP32 Pin |
|---|---|---|
| OLED (128x64, I2C) | SDA | A4 |
| OLED (128x64, I2C) | SCL | A5 |
| OLED (128x64, I2C) | VDD | VBUS |
| OLED (128x64, I2C) | GND | GND |
| Pressure Sensor | OUT | A0 |
| Pressure Sensor| VCC | 3.3V |
| Pressure Sensor| GND | GND |
| Thermistor (ATC Semitec 104GT-2) | One end | 3.3V |
| Thermistor (ATC Semitec 104GT-2) | Other end (sense node) | A1 |
| Resistor 100k (fixed divider resistor) | One end (sense node) | A1 |
| Resistor 100k (fixed divider resistor) | Other end | GND |
| Power Supply (5V DC) | Input | USB-C |

> Important: power the board via USB-C (5V input). Keep all sensor signals within 0–3.3V at the ADC pins.

## Functional Schematic (Mermaid)

```mermaid
flowchart LR
    PS[5V DC Supply] -->|USB-C| NANO[Arduino Nano ESP32]
    
    subgraph ARDINO[Arduino Nano ESP32]
      NANO --> GND[(Common GND)]
      NANO --> V33[3.3V]
      NANO --> A0[A0]
      NANO --> A1[A1]
      NANO --> A4[A4 SDA]
      NANO --> A5[A5 SCL]
      NANO --> VBUS[VBUS 5V DC]
    end 

    subgraph OLED[OLED 128x64 I2C]
      O_SDA[SDA]
      O_SCL[SCL]
      O_VDD[VDD]
      O_GND[GND]
    end

    A4[A4 SDA] --- O_SDA
    A5[A5 SCL] --- O_SCL
    VBUS[VBUS 5V DC] --- O_VDD
    GND --- O_GND

    subgraph PRESS[Pressure Sensor]
      P_OUT[Signal]
      P_VCC[VCC]
      P_GND[GND]
    end

    A0[A0] --- P_OUT
    V33[3.3V] --- P_VCC
    GND --- P_GND

    subgraph TEMP[NTC Thermistor Divider]
      V33 --> TH[104GT-2 Thermistor]
      TH --> A1[A1 Sense Node]
      A1 --> R100[100k Resistor]
      R100 --> GND
    end
```

## Wiring Checklist

- Shared ground between **all** modules.
- Keep analog sensor wires short and routed away from high-voltage lines.
- If readings are noisy, add a small RC filter on A0/A1 (e.g., 1k + 100nF) near the Nano ESP32.
- If the pressure sensor is powered from 3.3V, its output span is typically lower than at 5V; re-calibrate pressure conversion in firmware.

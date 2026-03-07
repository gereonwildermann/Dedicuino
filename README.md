# Espresso Machine Display – Arduino Nano ESP32 Project

> **Note**: This is a reimplementation and enhancement of the original [DeLonghi Dedica EC885/EC685 modification by CaiJonas](https://github.com/CaiJonas/DeLonghi-Dedica-EC885-EC685-modification/). This fork uses Arduino Nano ESP32, adds web dashboard, OTA updates, calibration tools, and 3.3V sensor architecture.

With this project, you can monitor and visualize the coffee brewing and extraction process by installing a pressure sensor and a temperature sensor. No modifications are made to the machine's electronics — it is purely a non-invasive monitoring and display system shown on a small screen. This helps in determining the optimal grind size and coffee dose.

For installation, you only need to disconnect the silicone hose between the thermoblock and the outlet. This hose is available as a spare part, so the machine can be fully restored to its original condition at any time. The display is designed in a way that no drilling is required — it uses the original mounting holes of the logo badge.

The system is based on an Arduino Nano ESP32 powered via USB-C (5V). It can be connected without any soldering or modifications to the original circuit board.


---

## ✨ Features

- **Non-invasive monitoring**: The machine remains unmodified — no soldering to the maschine electronic, no drilling, and 100% reversible installation.
- **Real-time shot timer**: Accurately displays extraction duration from the moment the pressure is high enough and the extraction starts.
- **Pressure measurement**: Displays current and maximum brewing pressure during extraction, afterwards maximum and average pressure.
- **Temperature display**: Shows current heater block temperature while brewing, afterwards maximum and average temperature.
- **Post-shot analysis**: After extraction, the display shows average & maximum pressure and temperature values.
- **OLED display**: Compact 128x64 I2C screen for clear and minimalistic visualization.
- **Web dashboard**: Live monitoring via Wi-Fi with auto-refreshing web interface.
- **OTA updates**: Wireless firmware updates over Wi-Fi.
- **Standalone system**: Powered by USB-C 5V, does not interfere with the machine's electronics.
- **3D-printed mounting**: Reuses existing logo mounting holes — no drilling required.

---


## 📦 Bill of Materials (BOM)

| **#** | **Component**                                   | **Quantity** | **Specification**                                              | **Link**                    |
|-------|--------------------------------------------------|--------------|----------------------------------------------------------------|-----------------------------|
| 1     | **Arduino Nano ESP32**                           | 1            | Microcontroller                                                 | [Arduino Nano ESP32 – Arduino Store](https://store.arduino.cc/products/nano-esp32) |
| 2     | **OLED Display**                                 | 1            | 0.96" I2C OLED Display, 128x64                                  | [DIYmall 0.96" OLED Display – Amazon](https://www.amazon.com/DIYmall-Serial-128x64-Display-Arduino/dp/B00O2KDQBE) |
| 3     | **Resistor (100kΩ)**                             | 1            | 100kΩ, 1/4W                                                     | [100kΩ Resistor 20-pack – LED-Shop](https://www.led-shop.com/Widerstand-1-4W-100k-Ohm-20er-Pack) |
| 4     | **Thermistor (ATC Semitec 104GT-2)**             | 1            | 100kΩ @ 25°C (NTC)                                              | [JINXIUS 104GT-2 Thermistor – Amazon](https://www.amazon.com/JINXIUS-Temperature-104NT-4-R025H42G-Thermistor-Compatible/dp/B097PBSQYZ) |
| 5     | **Pressure Sensor G1/4" (0-300 PSI)**            | 1            | Stainless steel, 0–300 PSI, analog output, 3.3V compatible      | [Pressure Transmitter G1/4" – Amazon](https://www.amazon.de/Drucktransmitter-Analogsensor-Wasser-Luftkompressor-0-300/dp/B0DPQTX1JR) |
| 6     | **T-Connector G1/4"-4mm, IQS-MSV**               | 1            | G1/4" female thread, 4mm hose connector                         | [T-Connector G1/4"-4mm – Landefeld](https://www.landefeld.de/artikel/de/t-steckanschluss-innengew-g-14-4mm-iqs-msv-standard-/IQSTFF%20144%20MSV) |
| 7     | **Nano ESP32-compatible IO breakout/shield**     | 1            | Optional breakout/shield for easier wiring                      | |
| 8     | **USB-C Cable**                                  | 1            | USB-C cable for 5V power supply                                 | |
| 9     | **Cables**                                       | -            | Various lengths for sensor connections                          | |
| 10    | **Screws M2.5x12mm**                             | 2            | M2.5, length: 12mm                                              | [M2.5x12mm Screws – Amazon](https://www.amazon.de/Innensechskant-Au%C3%9Fengewinde-Edelstahl-Anti-Lose-Maschinenbefestigungen-M2-5x12mm/dp/B0BJPLYXK1) |
| 11    | **Screw M4x12mm**                                | 1            | M4, length: 12mm                                                | [M4x12mm Screw – Amazon](https://www.amazon.de/dp/B08BL9PK4N) |
| 12    | **M2.5 Thread Inserts / T-nuts**                 | 2            | M2.5                                                            | [M2.5 Thread Inserts / T-nuts – Ahltec](https://www.ahltec.de/shop/de/M2-5-Gewindeeinsaetze--Einschlagmuttern-438.html) |
| 13    | **Thermal Paste**                                | 1            | Standard CPU-compatible thermal paste                           | [ARCTIC MX-4 Thermal Paste – Amazon](https://www.amazon.de/dp/B0795DP124) |



replacement hose: https://komtra.de/delonghi-ersatzteile/delonghi-ersatzteile/schlauch-l230-mm-klemmverbindung-klemmverbindung.html

---

## 🔧 Wiring Diagram & Setup Instructions

Detailed wiring is documented in [hardware/WIRING_DIAGRAM.md](hardware/WIRING_DIAGRAM.md).

### 🖥️ OLED Display (I2C)
- **SDA** → Arduino **A4**
- **SCL** → Arduino **A5**
- **GND** → Arduino **GND**
- **VCC** → Arduino **VBUS** (5V from USB)

### 🌡️ Thermistor (ATC Semitec 104GT-2) & Voltage Divider
- **Thermistor**:
  - One end → Arduino **3.3V**
  - Other end → Arduino **A1**
- **100kΩ Resistor**:
  - One end → Arduino **A1**
  - Other end → Arduino **GND**

### 📈 Pressure Sensor
- **VCC** → Arduino **3.3V**
- **GND** → Arduino **GND**
- **SIGNAL** → Arduino **A0**

### ⚡ Power Supply
- **USB-C** → Arduino **USB-C port** (5V input)

> ⚠️ **Note**: Sensors are powered from **3.3V rail**, not 5V. OLED uses **VBUS (5V)** for proper brightness. After switching to 3.3V sensors, recalibration is required.
  
---

## 🧑‍💻 Installation

### 🔐 Wi-Fi credentials (safe for GitHub)

For ESP32 builds, Wi-Fi credentials are read from a local file:

- Create `firmware/dedicuino_reimplementation/wifi_secrets.h`
- Add:
   - `#define WIFI_SSID "your_ssid"`
   - `#define WIFI_PASSWORD "your_password"`
   - `#define WIFI_OTA_HOSTNAME "dedicuino"` *(optional)*
   - `#define WIFI_OTA_PASSWORD "your_ota_password"` *(optional, empty = no OTA auth)*

Use `wifi_secrets.example.h` as template. `wifi_secrets.h` is ignored by git and will not be pushed.

### 🏷️ Private branding (local-only)

The repository stays public-safe by default (no private brand assets committed).

- Optional local splash logo file: `firmware/dedicuino_reimplementation/private_brand_logo.h`
- Template: `firmware/dedicuino_reimplementation/private_brand_logo.example.h`
- `private_brand_logo.h` is gitignored and will not be pushed to GitHub.

If `private_brand_logo.h` exists, firmware shows a third splash logo at startup.

### 📡 OTA firmware updates (Arduino Nano ESP32)

After first USB upload, OTA is enabled automatically.

1. Power the device and ensure it connects to the same Wi-Fi network as your computer.
2. In Arduino IDE, select board **Arduino Nano ESP32**.
3. In **Tools → Port**, pick the network port named with your OTA hostname (default: `dedicuino`).
4. Upload as usual.

If OTA password is set, Arduino IDE will ask for it during upload.

### 🌐 Web interface (live dashboard)

The firmware now hosts a live dashboard on the ESP32:

1. Connect phone/PC to the same Wi-Fi network as Dedicuino.
2. Open browser and go to: `http://dedicuino.local/` (or use the board IP address).
3. The page auto-refreshes every 500 ms and shows:
   - current pressure and temperature
   - shot timer
   - cup fill progress animation value
   - avg/max values during and after extraction

### 🧪 Calibration

Standalone calibration sketches are available in [firmware/calibration/README.md](firmware/calibration/README.md):

- [Pressure calibration sketch](firmware/calibration/pressure_sensor_calibration/pressure_sensor_calibration.ino)
   - OLED shows only `RAW`, `PSI`, `BAR`
   - Tune `PRESSURE_OFFSET` and `PRESSURE_CAL_FACTOR`
- [Temperature calibration sketch](firmware/calibration/temperature_sensor_calibration/temperature_sensor_calibration.ino)
   - OLED shows only `RAW`, `OHM`, `TEMP C`
   - Tune thermistor constants

JSON telemetry endpoint: `http://dedicuino.local/api/status`

---

## 🖥️ **Firmware Upload**

### Arduino IDE Setup

1. **Install Arduino IDE** (2.0 or later recommended)
2. **Install Arduino Nano ESP32 board support**:
   - Go to **Tools** → **Board** → **Boards Manager**
   - Search for "Arduino ESP32" and install
3. **Open firmware**: `firmware/dedicuino_reimplementation/dedicuino_reimplementation.ino`
4. **Select board**: **Tools** → **Board** → **Arduino Nano ESP32**
5. **Select port**: **Tools** → **Port** → your USB port
6. **Upload**: Click upload button or **Sketch** → **Upload**

### Required Libraries

Install via **Tools** → **Manage Libraries**:
- Adafruit GFX Library
- Adafruit SSD1306
- ESPAsyncWebServer (ESP32 only)
- AsyncTCP (ESP32 only)

After first upload, you can use OTA updates over Wi-Fi (see OTA section above).

---

## 🖨️ 3D Printing the Enclosure

> **Note**: The 3D enclosure design is from the [original project by CaiJonas](https://github.com/CaiJonas/DeLonghi-Dedica-EC885-EC685-modification/).

1. **Download the 3D Model:**
   - The 3D model for the enclosure is available on **Printables**. You can download it directly from
     https://www.printables.com/model/1214912-delonghi-dedica-ec885-ec685-modification-display/files

2. **Printing Settings:**
   - **Material:** ASA, ABS or PETG 
   - **Layer Height:** 0.2 mm (standard for good quality).
   - **Infill:** 20–30% (enough for stability without wasting too much material).
   - **Supports:** is needed, you should place the display holder flat on the front side
   - **Brim:** is needed if you print with ASA or ABS
   - **Fuzzy skin:** is optional

### 🔩 **Assembly of the Enclosure:**

**Assembly:** The enclosure uses **M2.5x12mm screws** (included in the Bill of Materials) to securely attach the components.
1. Remove the DeLonghi emblem by using pliers to detach the clips from the inside. Afterward, the emblem can be removed.
2. The remaining holes on the left and right will be used as mounting points in conjunction with the M2.5 insert nuts. The nuts will be glued from the inside using some CA glue.
3. Pass the 4-wire display cable through the middle large hole from the outside.
4. Screw in the M2.5 screws loosely.
5. Slide the display through the display mount from the back. It was specifically designed so that the display fits perfectly.
6. Hang the mount onto the screws and then lightly tighten the screws from the front through the mount. The screws do not require high tightening torque.
7. Place the display onto the mount and clip the display cover onto the front. The display cover holds the display in place without any glue needed.

![Wiring Diagram](images/Folie1.JPG)
![Wiring Diagram](images/Folie2.JPG)

---

## 🌡️ Mounting the Temperature Sensor

The temperature sensor (NTC thermistor type 104GT-2) is attached directly to the thermoblock with an M3x12mm screw to provide accurate temperature readings during the extraction process.
I found a position where it's not needed to drill a hole. The hole is already there and the material of the thermoblock is so soft that it's not needed to cut a thread before. Ofcourse it's better to cut the thread with an M3 thread cutting tool. Be careful.

### 🛠️ Steps:

1. **Access the thermoblock**  
   I will upload a video of the whole process in the near future.

2. **Cut the thread**  
   Using an M4 thread cutting tool is the best option.  
   It is also possible to use the screw itself, but be careful!  
   Gently screw in the screw with small back-and-forth movements. During this process, the screw needs some axial pressure.  
   Make sure that the screw is screwed in straight.

3. **Optional: file/sand a hollow for a better fit of the temperature sensor**  
   For better contact and more accurate temperature readings, you can use a 3mm reamer to create a better fit between the temperature sensor and the heater block.  
   This step is not necessary.

4. **Apply thermal paste**  
   Apply thermal paste to the temperature sensor and to the heater block.  
   Simply wipe off any excess paste after mounting the sensor and tightening the screw.  

5. **Route the cable**  
   Guide the thermistor cable down to the electronic board, avoiding sharp edges or tight bends. Connect it to the Arduino as described in the **Wiring Diagram** section.

![Wiring Diagram](images/Folie3.JPG)
![Wiring Diagram](images/Folie4.JPG)
![Wiring Diagram](images/Folie5.JPG)
![Wiring Diagram](images/Folie6.JPG)


### 🔁 Reversibility

All steps are 100% reversible — the machine can be restored to its original condition at any time since no permanent modifications are made.
---

## 📸 Photos
_Images of the mounted system go here_

---

## 📺 Demo Video

<a href="https://youtu.be/2GCKHqg131g" target="_blank">
  <img src="https://img.youtube.com/vi/2GCKHqg131g/0.jpg" width="300"/>
</a>

<a href="https://youtube.com/shorts/OjNCzPk4PEM" target="_blank">
  <img src="https://img.youtube.com/vi/OjNCzPk4PEM/0.jpg" width="300"/>
</a>

<a href="https://youtube.com/shorts/8p8zOG2kiDQ" target="_blank">
  <img src="https://img.youtube.com/vi/8p8zOG2kiDQ/0.jpg" width="300"/>
</a>


---

## ⚖️ License

This project is based on the [original work by CaiJonas](https://github.com/CaiJonas/DeLonghi-Dedica-EC885-EC685-modification/), licensed for **private, non-commercial use only** (Copyright © 2025 Cai Jonas Schäperkötter).

This fork maintains the same license terms. See `LICENSE` for details or contact the original author at caijonas404@gmail.com for commercial licensing inquiries.

---

## 🙏 Credits

- **Original project**: [CaiJonas/DeLonghi-Dedica-EC885-EC685-modification](https://github.com/CaiJonas/DeLonghi-Dedica-EC885-EC685-modification/)
- **3D enclosure design**: CaiJonas
- **This fork**: Arduino Nano ESP32 reimplementation with web dashboard, OTA updates, and calibration tools

If you appreciate the original concept and hardware design, consider supporting CaiJonas:

[![Buy CaiJonas a Coffee](https://img.shields.io/badge/Buy%20CaiJonas%20a%20Coffee-FFDD00?logo=buymeacoffee&logoColor=black&style=for-the-badge)](https://www.buymeacoffee.com/caijonas404)

---

## 🤝 Contributing
Feel free to open issues or submit pull requests!

---

## ⚠️ **Warning / Disclaimer**

- This project involves working with high voltage, mechanical components, and fluid systems. Improper handling may result in electric shock, injury, or equipment damage.  
- Components are introduced into the machine's circulation system that come into contact with water. This is done at your own risk and may pose potential health hazards.  
- Modifying your machine will likely void any manufacturer warranty.  
- You assume full responsibility for any risks, damages, or consequences. Proceed only if you have adequate experience with electronics, high-voltage systems, and fluid-handling safety.


# M5Stack CoreS3 Multi-Gauge  
# M5Stack CoreS3 マルチメーター

A compact digital dashboard driven by **M5Stack CoreS3 + ADS1015** that displays:

* **Oil / Fuel / Boost Pressure** via **Defi PDF00903S** (0 – 10 bar, 0.5 – 4.5 V)  
* **Oil / Water Temperature** via **Defi PDF00703S** (–40 – 150 °C, 0.5 – 4.5 V)  
* **Engine RPM** via a 0–5 V pulse signal (frequency-to-voltage converter or direct digital capture)


<img src="https://github.com/user-attachments/assets/d3a8bfdc-0bba-4519-b64a-f31a1ec9a9f4" width="640px">


---

## 📘 日本語 README

### 概要
このプロジェクトは、**M5Stack CoreS3** と **ADS1015** ADC を用いて、Defi 製センサ **PDF00903S**・**PDF00703S** および車両の **回転数パルス信号** を表示する車載用マルチメーターです。  
サーキットでの簡易モニタリング用途に最適化しています。

### 主な機能
- 油圧・燃圧・ブースト (0–10 bar) 半円アナログメーター  
  - 本リポジトリでは **油圧**・**油温** を実装済み
- 油温 / 水温 (–40–150 °C) デジタル数値＋バー表示  
- 回転数：50ms 間隔でサンプリング、シフトランプ設定可能  
- 10Hz 更新のセンサログ出力（Serial または microSD）  
- 自動輝度調整（オプション／GC0308 ALS対応）  

### ハードウェア構成
| モジュール       | 型番 / 仕様                       | 備考                     |
|------------------|----------------------------------|--------------------------|
| MCU              | **M5Stack CoreS3** (ESP32-S3)    | USB-C, 2.0インチ IPS LCD |
| ADC              | **ADS1015** 12bit / I²C / 4ch     | アナログ入力             |
| 圧力センサ       | **PDF00903S** (Defi)              | CH0 / 0.5 – 4.5 V        |
| 温度センサ1      | **PDF00703S** (Defi)              | CH1 / 0.5 – 4.5 V        |
| 温度センサ2      | **PDF00703S** (Defi)              | CH2 / 0.5 – 4.5 V        |
| RPM入力信号      | レブランプ付き                                | CH3 / 0–5V パルス        |
| 電源             | 5V                               | CoreS3 USB経由           |

> 📌 詳しい配線図は後日追加予定です。

---

## English README

### Overview
This project turns an **M5Stack CoreS3** and **ADS1015 ADC** into a simple yet powerful multi-gauge that reads:

- **Oil / Fuel / Boost Pressure** using **Defi PDF00903S** (0.5–4.5 V, 0–10 bar)  
- **Oil / Water Temperature** using **Defi PDF00703S** (0.5–4.5 V, –40 to 150°C)  
- **Engine RPM** using a 0–5V pulse input  

Perfect for vintage cars lacking modern instrumentation or for lightweight track-day data monitoring.

### Features
- Semi-circular analog gauge (0–10 bar, pressure)
  - In this repository, **oil pressure** and **oil temperature** are implemented.
- Digital + bar graph temperature display  
- RPM sampling every 50 ms, with configurable shift-light trigger  
- 10 Hz data logging to Serial or SD  
- Optional ambient light auto dimming (via GC0308 ALS)  
- All parameters configurable in `config.h`  

### Hardware Configuration
| Module           | Part / Spec                    | Notes                   |
|------------------|-------------------------------|-------------------------|
| MCU              | **M5Stack CoreS3** (ESP32-S3)  | 2.0" IPS, USB-C         |
| ADC              | **ADS1015** 12-bit, I²C, 4ch    | Analog signal input     |
| Pressure Sensor  | **PDF00903S** (Defi)           | CH0, 0.5–4.5V           |
| Temp Sensor 1    | **PDF00703S** (Defi)           | CH1, 0.5–4.5V           |
| Temp Sensor 2    | **PDF00703S** (Defi)           | CH2, 0.5–4.5V           |
| RPM Input        | -                              | CH3, 0–5V pulse         |
| Power Supply     | 5V                             | Powered via USB         |

> 📌 Detailed wiring diagrams will be added soon.

---

### License
This project is licensed under the **MIT License**.  
Use in vehicles is at your own risk—always validate sensor readings before driving.

---

🚗 Built for performance, track use, and hobbyist tuning.

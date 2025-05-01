# M5Stack CoreS3 Multi-Gauge  
# M5Stack CoreS3 マルチメーター

A compact digital dashboard driven by **M5Stack CoreS3 + ADS1015** that displays:

* **Oil / Fuel / Boost Pressure** via **Defi PDF00903S** (0 – 10 bar, 0.5 – 4.5 V)  
* **Oil / Water Temperature** via **Defi PDF00703S** (–40 – 150 °C, 0.5 – 4.5 V)  
* **Engine RPM** via a 0–5 V pulse signal (frequency-to-voltage converter or direct digital capture)  

---

## 日本語 README

### 概要
M5Stack CoreS3 と ADS1015 ADC を用いて、Defi 製センサ **PDF00903S**・**PDF00703S** および ECU/ディストリビューター等からの **回転数パルス信号** をリアルタイム表示する車載メーターです。純正メーターが不足する車両の追加計器や、サーキット走行時の簡易データロガーとして利用できます。  
SVG や GUI テーマはすべて **M5GFX** で描画し、30 FPS を目標に最適化しています。

### 主な機能
- 油圧・燃圧・ブースト (0–10 bar) 半円アナログメーター  
- 油温 / 水温 (–40–150 °C) デジタル＋バー表示  
- 10 Hz 更新の数値ログ（Serial または microSD）  
- 回転数 50 ms サンプリング、シフトランプ点灯域を設定可能  
- 自動輝度調整（オプション／GC0308 ALS）  
- コンフィグは `config.h` でワンタッチ切替

### ハードウェア構成
| モジュール | 型番 / 仕様 | 備考 |
| ---------- | ---------- | ---- |
| MCU | **M5Stack CoreS3** (ESP32-S3, 2.0 インチ IPS) | USB-C, Grove 端子 |
| ADC | **ADS1015** 12 bit, I²C, 4ch | アナログセンサ用 |
| 圧力センサ | **PDF00903S** (Defi) | CH0 (0.5 – 4.5 V) |
| 温度センサ | **PDF00703S** (Defi) | CH1 (0.5 – 4.5 V) |
| RPM入力 | 0–5 V 方形波 → 分圧 3.3 V | CH3 or GPIO 3 |
| 電源 | 5 V (シガー/USB) | CoreS3 経由で給電 |

> **配線図**は `docs/wiring.svg` を参照してください。  
>   センサ GND と CoreS3 GND は一点接地するとノイズに強くなります。

### ソフトウェア要件
- **Arduino IDE 2.x** or **PlatformIO**  
- ESP32 Arduino 3.0.0 以降  
- ライブラリ: `M5CoreS3`, `M5GFX`, `Adafruit_ADS1X15`, `ESP32TimerInterrupt`

```bash
# PlatformIO ビルド例
pio run -e m5stack-cores3 -t upload -t monitor

# 配線例 / Wiring Example

## Mermaid 配線図 / Mermaid Wiring Diagram

```mermaid
graph LR
    subgraph Core
        CoreS3[M5Stack CoreS3]
    end
    subgraph ADC
        ADS1015[ADS1015]
    end
    subgraph Sensors
        OilTemp["PDF00703S<br>(油温)"]
        WaterTemp["PDF00703S<br>(水温)"]
        OilPress["PDF00903S<br>(油圧)"]
    end
    OilTemp -- A0 --> ADS1015
    WaterTemp -- A1 --> ADS1015
    OilPress -- A2 --> ADS1015
    ADS1015 -- GPIO9 SDA --> CoreS3
    ADS1015 -- GPIO8 SCL --> CoreS3
    CoreS3 ..> ADS1015 : 5V / GND
    CoreS3 ..> Sensors : 5V / GND
```

## M5Stack CoreS3 と ADS1015

| CoreS3 ピン | ADS1015 ピン | 説明 |
|-------------|--------------|------|
| 5V          | VDD          | 電源 (5V)
| GND         | GND          | 共通 GND
| GPIO9       | SDA          | I²C データ (Wire.begin(9, 8))
| GPIO8       | SCL          | I²C クロック

## ADS1015 とセンサー

| ADS1015 チャンネル | 接続センサー | 備考 |
|--------------------|--------------|------|
| A0                 | PDF00703S (油温) | 0.5–4.5 V アナログ出力
| A1                 | PDF00703S (水温) | 0.5–4.5 V アナログ出力
| A2                 | PDF00903S (油圧) | 0.5–4.5 V アナログ出力

各センサーの電源は 5V を使用し、GND を共通にしてください。

---

## Wiring Example (English)

### M5Stack CoreS3 to ADS1015

| CoreS3 Pin | ADS1015 Pin | Note |
|------------|-------------|------|
| 5V         | VDD         | Power (5V)
| GND        | GND         | Common ground
| GPIO9      | SDA         | I²C data (Wire.begin(9, 8))
| GPIO8      | SCL         | I²C clock

### ADS1015 to Sensors

| ADS1015 Channel | Sensor | Note |
|-----------------|--------|------|
| A0 | PDF00703S (Oil Temp) | 0.5–4.5 V analog output
| A1 | PDF00703S (Water Temp) | 0.5–4.5 V analog output
| A2 | PDF00903S (Oil Pressure) | 0.5–4.5 V analog output

Supply all sensors with 5V and share the ground line.

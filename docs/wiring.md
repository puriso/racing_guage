## M5Stack CoreS3 × 油圧・水温・油温センサ 配線図

```mermaid
graph LR
    %% ===== Devices =====
    subgraph Core
        CoreS3["M5Stack<br>CoreS3"]
    end

    subgraph ADC
        ADS1015["ADS1015<br>I²C&nbsp;ADC"]
    end

    subgraph Sensors
        OilTemp["PDF00703S<br>油温"]
        WaterTemp["PDF00703S<br>水温"]
        OilPress["PDF00903S<br>油圧"]
    end

    subgraph Resistors
        R1["R1 4.7 kΩ<br>プルアップ"]
        R2["R2 4.7 kΩ<br>プルアップ"]
    end

    %% ===== Analog channels =====
    OilTemp  -- "A0" --> ADS1015
    WaterTemp -- "A1" --> ADS1015
    OilPress -- "A2" --> ADS1015

    %% ===== I²C bus =====
    ADS1015 -- "SDA" --> CoreS3
    ADS1015 -- "SCL" --> CoreS3

    %% Pull-ups to 3.3 V
    R1 -- "→ SDA" --> ADS1015
    R2 -- "→ SCL" --> ADS1015
    CoreS3 -.->|3.3 V| R1
    CoreS3 -.->|3.3 V| R2

    %% ===== Power rails =====
  CoreS3 -.->|5 V| ADS1015
    CoreS3 -.->|GND| ADS1015

  ADS1015 -.->|5 V| OilTemp
  ADS1015 -.->|5 V| WaterTemp
  ADS1015 -.->|5 V| OilPress

    ADS1015 -.->|GND| OilTemp
    ADS1015 -.->|GND| WaterTemp
    ADS1015 -.->|GND| OilPress
```

センサは 0.5 – 4.5 V の ratiometric 出力、ADS1015 は単電源 5 V 動作を想定。

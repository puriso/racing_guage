#!/bin/bash
set -e

# スタブヘッダーを使用してテストをビルド
g++ -Iinclude -Itest/stubs -Isrc -std=c++17 test/test_sensor.cpp src/modules/sensor.cpp -o test/test_sensor
# 実行でユニットテストをパス
./test/test_sensor

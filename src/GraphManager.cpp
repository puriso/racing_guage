#include "GraphManager.h"

GraphManager::GraphManager(M5Canvas& canvas, int width, int height)
  : _canvas(canvas), _width(width), _height(height) {
  _graphData = new float[width];
  initializeGraphData();
}

void GraphManager::initializeGraphData() {
  for (int i = 0; i < _width; i++) {
    _graphData[i] = _height;  // 初期値
  }
}

void GraphManager::drawScrollingLineGraph(float newValue) {
  _canvas.createSprite(_width, _height);
  float scaledValue = _height - ((newValue / 9.9) * _height);

  for (int i = 0; i < _width - 1; i++) {
    _graphData[i] = _graphData[i + 1];
  }
  _graphData[_width - 1] = scaledValue;

  // 背景をクリア
  _canvas.fillSprite(0x18E3);

  // 折れ線グラフを描画
  for (int i = 1; i < _width; i++) {
    float startValue = 9.9 - (_graphData[i - 1] / _height) * 9.9;
    uint16_t color = (startValue <= 2.0) ? 0xBDF7 : (startValue >= 8.0) ? RED : WHITE;
    _canvas.drawLine(i - 1, _graphData[i - 1], i, _graphData[i], color);
  }

  // 目盛り線とラベルの描画
  uint16_t faintLineColor = 0x4208; // 薄いグレー
  int padding = 4;                 // ラベルと線の間のパディング

  // 配列番号 に対応する目盛り線とラベルを描画
  for (int value : {2, 8}) {
    int y = _height - ((_height / 9.9) * value); // 値に対応する高さを計算

    // 水平線（破線風）を描画
    for (int x = 0; x < _width; x += 4) {
      _canvas.drawPixel(x, y, faintLineColor);
    }
  }

  // ラベルを描画（折れ線グラフの後に描画）
  for (int value : {2, 5, 8}) {
    int y = _height - ((_height / 9.9) * value); // 値に対応する高さを計算

    // 数字ラベルを描画（位置を自動調整）
    char label[5];
    snprintf(label, sizeof(label), "%d", value);
    _canvas.setTextFont(1);
    _canvas.setTextColor(WHITE, 0x18E3);

    int labelY = y - padding - 8; // ラベルを線の上に配置
    if (labelY < 0) {
      labelY = y + padding; // 画面の上端を超える場合は線の下に配置
    } else if (labelY + 8 > _height) {
      labelY = y - padding - 8; // 下端の場合も線の上
    }
    _canvas.setCursor(2, labelY); // ラベル位置を設定
    _canvas.print(label);
  }

  // スプライトを画面に描画
  _canvas.pushSprite(0, 200);
}

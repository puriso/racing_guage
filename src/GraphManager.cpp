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

  _canvas.fillSprite(BLACK);
  for (int i = 1; i < _width; i++) {
    float startValue = 9.9 - (_graphData[i - 1] / _height) * 9.9;
    uint16_t color = (startValue <= 2.0) ? 0xBDF7 : (startValue >= 8.0) ? RED : WHITE;
    _canvas.drawLine(i - 1, _graphData[i - 1], i, _graphData[i], color);
  }
  _canvas.pushSprite(0, 200);
}

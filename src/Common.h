// テキスト中央配置X座標計算
int16_t calculateCenteredX(int16_t spriteWidth, const char* text, M5Canvas& canvas) {
  return (spriteWidth - canvas.textWidth(text)) / 2;
}

// 平均計算関数
float calculateAverage(float values[], int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += values[i];
  }
  return sum / size;
}

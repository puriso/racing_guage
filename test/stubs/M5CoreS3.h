#ifndef M5CORES3_H
#define M5CORES3_H
class Ltr553Class {
 public:
  void begin(void*) {}
  int getAlsValue() { return 0; }
};
class CoreS3Class {
 public:
  Ltr553Class Ltr553;
};
extern CoreS3Class CoreS3;
#endif

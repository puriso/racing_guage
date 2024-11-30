#ifndef GRAPH_MANAGER_H
#define GRAPH_MANAGER_H

#include <M5GFX.h>

class GraphManager
{
 public:
  GraphManager(M5Canvas& canvas, int width, int height);
  void initializeGraphData();
  void drawScrollingLineGraph(float newValue);

 private:
  M5Canvas& _canvas;
  int _width;
  int _height;
  float* _graphData;
};

#endif

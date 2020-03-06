#pragma once
#include <stdint.h>

class Point {
 public:
  uint8_t x, y;
  Point() {
    x = 0;
    y = 0;
  }

  Point(uint8_t a, uint8_t b) {
    x = a;
    y = b;
  }
};
#pragma once

#include <matrix.h>

void print_temperature(Matrix mat, int m, int n) {
  const char* colors = " .-:=+*#%@";
  const int numColors = 10;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;

  // set the 'render' resolution
  const int h = 30;
  const int w = 50;

  // step size in each dimension
  const int sh = m / h;
  const int sw = n / w;

  // upper wall
  for (size_t i = 0; i < w + 2; i++) {
    printf("X");
  }
  printf("\n");

  // room
  for (size_t i = 0; i < h; i++) {
    // left wall
    printf("X");
    // actual room
    for (size_t j = 0; j < w; j++) {
      // get max temperature in this tile
      value_t max_t = 0;
      for (size_t x = sh * i; x < sh * i + sh; x++) {
        for (size_t y = sw * j; y < sw * j + sw; y++) {
          max_t = (max_t < mat[x * m + y]) ? mat[x * m + y] : max_t;
        }
      }
      value_t temp = max_t;

      // pick the 'color'
      int c = ((temp - min) / (max - min)) * numColors;
      c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

      // print the average temperature
      printf("%c", colors[c]);
    }
    // right wall
    printf("X\n");
  }

  // lower wall
  for (size_t i = 0; i < w + 2; i++) {
    printf("X");
  }
  printf("\n");
}

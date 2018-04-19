#pragma once

#include <matrix.h>

void print_temperature(Matrix m, int N, int M) {
  const char* colors = " .-:=+*#%@";
  const int numColors = 10;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;

  // set the 'render' resolution
  int H = 30;
  int W = 50;

  // step size in each dimension
  int sH = N / H;
  int sW = M / W;

  // upper wall
  for (int i = 0; i < W + 2; i++) {
    printf("X");
  }
  printf("\n");

  // room
  for (int i = 0; i < H; i++) {
    // left wall
    printf("X");
    // actual room
    for (int j = 0; j < W; j++) {
      // get max temperature in this tile
      value_t max_t = 0;
      for (int x = sH * i; x < sH * i + sH; x++) {
        for (int y = sW * j; y < sW * j + sW; y++) {
          max_t = (max_t < m[x * N + y]) ? m[x * N + y] : max_t;
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
  for (int i = 0; i < W + 2; i++) {
    printf("X");
  }
  printf("\n");
}

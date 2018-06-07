#include "extend_to_multiple.h"

kernel void mat_mul(const global const float* a, const global const float* b, global float* c, local float* sub_a, local float* sub_b, const unsigned long m, const unsigned long k, const unsigned long n) {
  const size_t row = get_local_id(0);
  const size_t col = get_local_id(1);
  const size_t global_row = get_global_id(0);
  const size_t global_col = get_global_id(1);

  float acc = 0.0f;

  const size_t tile_size = get_local_size(0);

  const int num_tiles = extend_to_multiple(k, tile_size) / tile_size;

  for (size_t t = 0; t < num_tiles; t++) {
    const size_t tiled_row = tile_size * t + row;
    const size_t tiled_col = tile_size * t + col;

    if (tiled_col < k) {
      sub_a[col * tile_size + row] = a[global_row * k + tiled_col];
    } else {
      sub_a[col * tile_size + row] = 0.0f;
    }

    if (tiled_row < k) {
      sub_b[row * tile_size + col] = b[tiled_row * n + global_col];
    } else {
      sub_b[row * tile_size + col] = 0.0f;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int ti = 0; ti < tile_size; ti++) {
      acc += sub_a[ti * tile_size + row] * sub_b[ti * tile_size + col];
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if (global_row < m && global_col < n) {
    c[global_row * n + global_col] = acc;
  }
}

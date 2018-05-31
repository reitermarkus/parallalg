kernel void mat_mul(const global const float* a, const global const float* b, global float* c, local float* sub_a, local float* sub_b, const unsigned long m, const unsigned long k, const unsigned long n) {
  const int row = get_local_id(0);
  const int col = get_local_id(1);
  const int global_row = get_global_id(0);
  const int global_col = get_global_id(1);

  float acc = 0.0f;

  const size_t tile_size = get_local_size(0);

  const int num_tiles = k / tile_size;

  for (int t = 0; t < num_tiles; t++) {
    const int tiled_row = tile_size * t + row;
    const int tiled_col = tile_size * t + col;

    if (global_row < m && global_col < n) {
      sub_a[row * tile_size + col] = a[global_row * k + tiled_col];
      sub_b[row * tile_size + col] = b[tiled_row * n + global_col];
    } else {
      sub_a[row * tile_size + col] = 0.0f;
      sub_b[row * tile_size + col] = 0.0f;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int ti = 0; ti < tile_size; ti++) {
      acc += sub_a[row * tile_size + ti] * sub_b[ti * tile_size + col];
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if (global_row < m && global_col < n) {
    c[global_row * m + global_col] = acc;
  }
}

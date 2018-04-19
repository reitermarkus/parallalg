__kernel void calc_temp(__global float const* matrix_a, __global float* matrix_b, __local float* t, ulong n, ulong source_x, ulong source_y) {
  size_t i = get_global_id(0);
  size_t j = get_global_id(1);

  size_t w = get_local_size(0);

  size_t li = get_local_id(0);
  size_t lj = get_local_id(1);

  size_t bs = w + 2; // local buffer size is work-group size + 2

  float tc = t[(li + 1) * bs + (lj + 1)] = matrix_a[i * n + j];

  // Get temperatures left/right.
  if (lj == 0) {
    t[(li + 1) * bs + (lj + 0)] = j !=  0    ? matrix_a[i * n + (j - 1)] : tc;
  } else if (lj == w - 1) {
    t[(li + 1) * bs + (lj + 2)] = j != n - 1 ? matrix_a[i * n + (j + 1)] : tc;
  }

  // Get temperatures up/down.
  if (li == 0) {
    t[(li + 0) * bs + (lj + 1)] = i !=  0    ? matrix_a[(i - 1) * n + j] : tc; // up
  } else if (li == w - 1) {
    t[(li + 2) * bs + (lj + 1)] = i != n - 1 ? matrix_a[(i + 1) * n + j] : tc; // down
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  // Center stays constant (the heat is still on).
  if (i == source_x && j == source_y) {
    matrix_b[i * n + j] = tc;
    return;
  }

  // Update temperature at current point.
  matrix_b[i * n + j] = tc + 0.2f * (
    t[(li + 1) * bs + (lj + 0)] +
    t[(li + 1) * bs + (lj + 2)] +
    t[(li + 0) * bs + (lj + 1)] +
    t[(li + 2) * bs + (lj + 1)] +
    (-4.0f * tc)
  );
}

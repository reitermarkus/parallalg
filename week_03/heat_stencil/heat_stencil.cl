__kernel void calc_temp(__global float const* matrix_a, __global float* matrix_b, size_t n, size_t source_x, size_t source_y) {
  size_t i = get_global_id(0);
  size_t j = get_global_id(1);

  // Center stays constant (the heat is still on).
  if (i == source_x && j == source_y) {
    matrix_b[i * n + j] = matrix_a[i * n + j];
  } else {
    // Get current temperature at (i,j).
    float tc = matrix_a[i * n + j];

    // Get temperatures left/right and up/down.
    float tl = j !=  0     ? matrix_a[i * n + (j - 1)] : tc;
    float tr = j != n - 1  ? matrix_a[i * n + (j + 1)] : tc;
    float tu = i !=  0     ? matrix_a[(i - 1) * n + j] : tc;
    float td = i != n - 1  ? matrix_a[(i + 1) * n + j] : tc;

    // Update temperature at current point.
    matrix_b[i * n + j] = tc + 0.2 * (tl + tr + tu + td + (-4.0 * tc));
  }
}

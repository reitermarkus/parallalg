kernel void calc_temp(global float const* matrix_a, global float* matrix_b, local float* t, ulong n, ulong source_x, ulong source_y) {
  size_t i = get_global_id(0);
  size_t j = get_global_id(1);

  size_t w = get_local_size(0);
  size_t h = get_local_size(1);

  size_t li = get_local_id(0);
  size_t lj = get_local_id(1);

  size_t bs = w + 2;

  #define G(X,Y) matrix_a[   (X) * n +   (Y)  ]
  #define L(X,Y) t[((X)+1)*bs + ((Y)+1)]

  if (i < n && j < n) {
    // Get center.
    L(li,lj) = G(i,j);

    // Get current temperatures left/right.
    if (li ==    0  ) L(li - 1, lj) = G(i - 1, j);
    if (li == w - 1) L(li + 1, lj) = G(i + 1, j);

    // Get current temperatures up/down.
    if (lj ==    0  ) L(li, lj - 1) = G(i, j - 1);
    if (lj == h - 1) L(li, lj + 1) = G(i, j + 1);
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  if (i >= n || j >= n) return;

  // Center stays constant (the heat is still on).
  if (i == source_x && j == source_y) {
    matrix_b[i * n + j] = L(li, lj);
    return;
  }

  // Get current temperature at (i, j).
  float tc = L(li, lj);

  // Get temperatures left/right and up/down.
  float tl = (j !=   0  ) ? L(li, lj - 1) : tc;
  float tr = (j != n - 1) ? L(li, lj + 1) : tc;
  float tu = (i !=   0  ) ? L(li - 1, lj) : tc;
  float td = (i != n - 1) ? L(li + 1, lj) : tc;

  // Update temperature at current point.
  matrix_b[i * n + j] = tc + 0.2f * (tl + tr + tu + td + (-4.0f * tc));
}

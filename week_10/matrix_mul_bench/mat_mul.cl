kernel void mat_mul(const global const float* a, const global const float* b, global float* c, local float* sub_a, local float* sub_b, const unsigned long m, const unsigned long k, const unsigned long n) {
  size_t row = get_global_id(0);
  size_t col = get_global_id(1);

  if (row >= m || col >= n) return;

  float acc = 0;

  for(int ki = 0; ki < k; ki++) {
    acc += a[row * k + ki] * b[ki * n + col];
  }

  c[row * n + col] = acc;
}

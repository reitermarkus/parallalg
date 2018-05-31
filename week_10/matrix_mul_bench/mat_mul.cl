kernel void mat_mul(const global const float* a, const global const float* b, global float* c, int N) {
  size_t i = get_global_id(1);
  size_t j = get_global_id(0);

  if (i >= N || j >= N) return;

  float sum = 0;
  for(int k = 0; k < N; k++) {
    sum += a[i * N + k] * b[k * N + j];
  }

  c[i * N + j] = sum;
}

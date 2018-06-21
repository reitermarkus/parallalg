kernel void iterate_cells(const unsigned long n, const unsigned long block_size, const unsigned long bi, const unsigned long bj, global unsigned long* minimum_costs, const global const unsigned long* sizes) {
  // get lower-left corner of current blocks
  unsigned long ci = (bi + 1) * block_size - 1;
  unsigned long cj = bj * block_size;

  // process current block in wave-front order
  for (int d = 0; d < 2 * block_size - 1; d++) {
    for (
      unsigned long li = d >= block_size ? block_size - 1 : d,
          lj = d < block_size ? 0 : d - block_size + 1;
      li >= 0 && lj < block_size;
      lj++, li--
    ) {
      // get coordinated in C
      unsigned long i = ci - li;
      unsigned long j = cj + lj;

      // check whether the current cell still of interest
      if (i > j || i >= n || j >= n) continue;

      if (i == j) {
        minimum_costs[i * n + j] = 0;
        continue;
      }

      // find cheapest cut between i and j
      unsigned long min = INT_MAX;
      for (unsigned long k = i; k < j; k++) {
        unsigned long costs = minimum_costs[i * n + k] + minimum_costs[(k + 1) * n + j] + sizes[i] * sizes[k + 1] * sizes[j + 1];
        if (costs < min) min = costs;
      }

      minimum_costs[i * n + j] = min;
    }
  }
}

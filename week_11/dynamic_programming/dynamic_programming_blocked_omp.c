#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void iterate_cells(unsigned long n, unsigned long block_size, unsigned long bi, unsigned long bj, unsigned long* minimum_costs, unsigned long* sizes) {
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

void iterate_blocks(unsigned long n, unsigned long block_size, unsigned long* minimum_costs, unsigned long* sizes) {
  unsigned long num_blocks = ceil(n / (double)block_size);

  // iterate through blocks in wave-front order
  for (unsigned long d = 0; d < num_blocks; d++) {
    #pragma omp parallel for
    for (unsigned long i = 0; i < num_blocks - d; i++) {
      unsigned long j = i + d;
      iterate_cells(n, block_size, i, j, minimum_costs, sizes);
    }
  }
}

int main(int argc, char** argv) {
  unsigned long min_size = 10;
  unsigned long max_size = 20;

  // read problem size
  unsigned long n          = argc >= 2 ? atoi(argv[1]) : 2000;
  unsigned long block_size = argc >= 3 ? atoi(argv[2]) : 22;

  printf("Computing minimum cost for multiplying %lu matrices ...\n", n);

  // generate random matrix sizes
  srand(0);
  unsigned long s = n + 1;
  unsigned long* sizes = malloc(sizeof(*sizes) * s);
  for (unsigned long i = 0; i < s; i++) {
    sizes[i] = ((rand() / (float)RAND_MAX) * (max_size - min_size)) + min_size;
  }

  // compute minimum costs
  unsigned long* minimum_costs = malloc(sizeof(*minimum_costs) * n * n);

  double start = now();

  iterate_blocks(n, block_size, minimum_costs, sizes);

  double end = now();

  printf("Minimal costs: %lu FLOPS\n", minimum_costs[0 * n + n - 1]);
  printf("Total time: %.3fs\n", (end - start));

  // clean
  free(minimum_costs);
  free(sizes);

  // done
  return EXIT_SUCCESS;
}

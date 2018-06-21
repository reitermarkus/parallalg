#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char** argv) {
  int min_size = 10;
  int max_size = 20;

  // read problem size
  int n          = argc >= 2 ? atoi(argv[1]) : 2000;
  int block_size = argc >= 3 ? atoi(argv[2]) : 22;

  int s = n + 1;
  printf("Computing minimum cost for multiplying %d matrices ...\n", n);

  // generate random matrix sizes
  srand(0);
  int* l = malloc(sizeof(int) * s);
  for (int i = 0; i < s; i++) {
    l[i] = ((rand() / (float)RAND_MAX) * (max_size - min_size)) + min_size;
  }

  // compute minimum costs
  int* minimum_costs = malloc(sizeof(*minimum_costs) * n * n);

  double start = now();

  // initialize solutions for costs of single matrix
  for (int i = 0; i < n; i++) {
    minimum_costs[i * n + i] = 0; // there is no multiplication cost for those sub-terms
  }

  int num_blocks = ceil(n / (double)block_size);

  // iterate through blocks in wave-front order
  for(int bd = 0; bd < num_blocks; bd++) {
    #pragma omp parallel for
    for(int bi = 0; bi < num_blocks - bd; bi++) {
      int bj = bi + bd;

     // get lower-left corner of current blocks
      int ci = (bi + 1) * block_size - 1;
      int cj = bj * block_size;

      // process current block in wave-front order
      for(int d = 0; d < 2 * block_size - 1; d++) {
        for(int li = d >= block_size ? block_size - 1 : d,
            lj = d < block_size ? 0 : d - block_size + 1; li >= 0 && lj < block_size; lj++, li--) {
          // get coordinated in C
          int i = ci - li;
          int j = cj + lj;

          // check whether the current cell still of interest
          if (i > j || i >= n || j >= n)
            continue;

          if (i == j) {
            minimum_costs[i * n + j] = 0;
            continue;
          }

          // find cheapest cut between i and j
          int min = INT_MAX;
          for(int k = i; k < j; k++) {
            int costs = minimum_costs[i * n + k] + minimum_costs[(k + 1) * n + j] + l[i] * l[k+1] * l[j+1];
            min = costs < min ? costs : min;
          }

          minimum_costs[i * n + j] = min;
        }
      }
    }
  }

  double end = now();

  printf("Minimal costs: %d FLOPS\n", minimum_costs[0 * n + n - 1]);
  printf("Total time: %.3fs\n", (end - start));

  // clean
  free(minimum_costs);
  free(l);

  // done
  return EXIT_SUCCESS;
}

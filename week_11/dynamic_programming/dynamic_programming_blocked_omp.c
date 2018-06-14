
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
  int n = 2000;
  int num_tiles = 5;

  switch (argc) {
    case 2:
      n = atoi(argv[1]);
      break;
    case 3:
      num_tiles = atoi(argv[2]);
      break;
  }

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

  // compute minimal costs for multiplying A_i x ... x A_j
  for (int d = 1; d < n; d++) {   // < distance between i and j
    #pragma omp parallel for
    for (int i = 0; i < n - d; i++) { // < starting at each i
      for(int q = 0; q < num_tiles; q++) {
        int j = i + d;              // < compute end j
        int tiled_row = num_tiles * q + j;
        int tiled_col = num_tiles * q + i;

        // find cheapest cut between i and j
        int min = INT_MAX;
        for (int k = tiled_col; k < tiled_row; k++) {
          int costs = minimum_costs[tiled_col * num_tiles + k] + minimum_costs[(k + 1) * num_tiles + tiled_row] + l[tiled_col] * l[k + 1] * l[tiled_row + 1];
          min = (costs < min) ? costs : min;
        }

        minimum_costs[tiled_col * num_tiles + tiled_row] = min;
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

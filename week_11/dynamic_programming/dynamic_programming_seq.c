
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
  if (argc > 1) {
    n = atoi(argv[1]);
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
    for (int i = 0; i < n - d; i++) { // < starting at each i
      int j = i + d;              // < compute end j

      // find cheapest cut between i and j
      int min = INT_MAX;
      for (int k = i; k < j; k++) {
        int costs = minimum_costs[i * n + k] + minimum_costs[(k + 1) * n + j] + l[i] * l[k + 1] * l[j + 1];
        min = (costs < min) ? costs : min;
      }

      minimum_costs[i * n + j] = min;
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

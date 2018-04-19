#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#include "matrix.h"
#include "common/helpers.h"

int main(int argc, char** argv) {
  // 'parsing' optional input parameter = problem size
  int N = 500;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 100;
  printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);

  // ---------- setup ----------

  // create a buffer for storing temperature fields
  Matrix A = create_matrix(N, N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      A[i * N + j] = 273; // temperature is 0°C everywhere (273K)
    }
  }

  // and there is a heat source in one corner
  int source_x = N / 4;
  int source_y = N / 4;
  A[source_x * N + source_y] = 273 + 60;

  printf("Initial:\n");
  print_temperature(A, N, N);

  // ---------- compute ----------

  // create a second buffer for the computation
  Matrix B = create_matrix(N, N);

  timestamp begin = now();

  // -- BEGIN ASSIGNMENT --

  // TODO: parallelize the following computation using OpenMP

  // for each time step ..
  for (int t = 0; t < T; t++) {
    // .. we propagate the temperature
    #pragma omp parallel for
    for (long long i = 0; i < N; i++) {
      for (long long j = 0; j < N; j++) {
        // center stays constant (the heat is still on)
        if (i == source_x && j == source_y) {
          B[i * N + j] = A[i * N + j];
          continue;
        }

        // get current temperature at (i,j)
        value_t tc = A[i * N + j];

        // get temperatures left/right and up/down
        value_t tl = (j !=  0 ) ? A[i * N + (j - 1)] : tc;
        value_t tr = (j != N-1) ? A[i * N + (j + 1)] : tc;
        value_t tu = (i !=  0 ) ? A[(i - 1) * N + j] : tc;
        value_t td = (i != N-1) ? A[(i + 1) * N + j] : tc;

        // update temperature at current point
        B[i * N + j] = tc + 0.2 * (tl + tr + tu + td + (-4 * tc));
      }
    }

    // swap matrixes (just pointers, not content)
    Matrix H = A;
    A = B;
    B = H;

    // show intermediate step
    if (!(t % 1000)) {
      printf("Step t=%d:\n", t);
      print_temperature(A, N, N);
    }
  }

  // -- END ASSIGNMENT --

  timestamp end = now();
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  free(B);

  // ---------- check ----------

  printf("Final:\n");
  print_temperature(A, N, N);

  bool success = true;
  for (long long i = 0; i < N; i++) {
    for (long long j = 0; j < N; j++) {
      value_t temp = A[i * N + j];
      if (273 <= temp && temp <= 273 + 60) continue;
      success = false;
      break;
    }
  }

  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ---------- cleanup ----------

  free(A);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <stdio.h>
#include "matrix.h"
#include "utils.h"

int main(int argc, char** argv) {
  int n = 1000;
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  printf("Matrix Multiplication with n=%d\n", n);

  // -------------------- SETUP -------------------- //
  Matrix mtx_a = create_matrix(n, n);
  Matrix mtx_b = create_matrix(n, n);

  fill_matrices(mtx_a, mtx_b, n, n);

  Matrix mtx_res = create_matrix(n, n);

  // -------------------- START -------------------- //
  timestamp begin = now();

  // ------------------- COMPUTE -------------------- //
  #pragma omp parallel for
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      value_t sum = 0;
      for (size_t k = 0; k < n; k++) {
        sum += mtx_a[i * n + k] * mtx_b[k * n + j];
      }
      mtx_res[i * n + j] = sum;
    }
  }

  timestamp end = now();
  // -------------------- END -------------------- //
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  // ------------------- CHECK ------------------- //
  bool success = check(mtx_res, n, n);
  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  free(mtx_a);
  free(mtx_b);
  free(mtx_res);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

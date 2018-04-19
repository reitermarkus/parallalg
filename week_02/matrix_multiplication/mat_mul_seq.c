#include <stdio.h>
#include <utils.h>
#include "matrix/helpers.h"
#include <matrix.h>

int main(int argc, char **argv) {

  if (argc > 1) {
    n = atoi(argv[1]);
  }

  printf("Matrix Multiplication with n=%d\n", n);

  // -------------------- SETUP -------------------- //
  init_matrices();

  // -------------------- START -------------------- //
  timestamp begin = now();

  // ------------------- COMPUTE -------------------- //
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
  bool success = check();
  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  release_matrix(mtx_a);
  release_matrix(mtx_b);
  release_matrix(mtx_res);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

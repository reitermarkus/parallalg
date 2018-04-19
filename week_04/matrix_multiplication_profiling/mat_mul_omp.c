#include <stdio.h>
#include <utils.h>
#include "matrix/helpers.h"
#include <matrix.h>

int main(int argc, char** argv) {
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  printf("Matrix Multiplication with n=%d\n", n);

  // -------------------- SETUP -------------------- //
  init_matrices();

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
  printf("Total time: %.3f ms\n", (end - begin) * 1000);

  double mflops = n / 1000000.0 * n * n * 2.0 / (double)(end - begin);
  printf("MFLOPS: %.3f\n", mflops);

  // ------------------- CHECK ------------------- //
  bool success = check();
  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  free(mtx_a);
  free(mtx_b);
  free(mtx_res);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

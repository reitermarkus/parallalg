#pragma once
#include <stdio.h>
#include <stdlib.h>

#include "common/headers.h"
#include "common/utils.h"
#include "matrix/matrix.h"

static int N = 1000;
static Matrix mtx_A;
static Matrix mtx_B;
static Matrix mtx_RES;

void init_matrices() {
  mtx_A = create_matrix(N, N);
  mtx_B = create_matrix(N, N);

  fill_matrices(mtx_A, mtx_B, N);

  mtx_RES = create_matrix(N, N);
}

bool check() {
  bool success = true;
  for (long long i = 0; i < N; i++) {
    for (long long j = 0; j < N; j++) {
      if (mtx_RES[i * N + j] == i * j)
        continue;
      success = false;
      break;
    }
  }

  return success;
}

int main(int argc, char **argv) {

  if (argc > 1) {
    N = atoi(argv[1]);
  }

  printf("Matrix Multiplication with N=%d\n", N);

  // -------------------- SETUP -------------------- //
  init_matrices();

  // -------------------- START -------------------- //
  timestamp begin = now();

  // ------------------- COMPUTE -------------------- //
  #pragma omp parallel for
  for (long long i = 0; i < N; i++) {
    for (long long j = 0; j < N; j++) {
      value_t sum = 0;
      for (long long k = 0; k < N; k++) {
        sum += mtx_A[i * N + k] * mtx_B[k * N + j];
      }
      mtx_RES[i * N + j] = sum;
    }
  }

  timestamp end = now();
  // -------------------- END -------------------- //
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  // ------------------- CHECK ------------------- //
  bool success = check();
  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  release_matrix(mtx_A);
  release_matrix(mtx_B);
  release_matrix(mtx_RES);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

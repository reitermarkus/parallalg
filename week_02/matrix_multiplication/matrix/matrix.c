#include "../common/headers.h"
#include "matrix.h"

Matrix create_matrix(int N, int M) {
  // create data and index vector
  return malloc(sizeof(value_t) * N * M);
}

void release_matrix(Matrix m) { free(m); }

void fill_matrices(Matrix A, Matrix B, int N) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      A[i * N + j] = i * j;             // some matrix - note: flattened indexing!
      B[i * N + j] = (i == j) ? 1 : 0;  // identity matrix
    }
  }
}

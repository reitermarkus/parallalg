#pragma once

#include <stdlib.h>

typedef float value_t;

typedef value_t* Matrix;

Matrix create_matrix(size_t n, size_t m) {
  return malloc(sizeof(value_t) * n * m);
}

void release_matrix(Matrix mat) {
  free(mat);
}

void fill_matrices(Matrix a, Matrix b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      a[i * n + j] = i * j;             // some matrix - note: flattened indexing!
      b[i * n + j] = (i == j) ? 1 : 0;  // identity matrix
    }
  }
}

void print_matrix(Matrix mat, size_t n, size_t m) {
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < m; j++) {
      printf("%f ", mat[i * n + j]);
    }
    printf("\n");
  }
}

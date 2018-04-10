#include "../common/headers.h"
#include "matrix.h"

int n = 100;

Matrix mtx_a;
Matrix mtx_b;
Matrix mtx_res;

void init_matrices() {
  mtx_a = create_matrix(n, n);
  mtx_b = create_matrix(n, n);

  fill_matrices(mtx_a, mtx_b, n);

  mtx_res = create_matrix(n, n);
}

bool check() {
  bool success = true;
  for (size_t i = 0; i < n; i++) {
    for (size_t j = 0; j < n; j++) {
      if (mtx_res[i * n + j] == i * j)
        continue;
      success = false;
      break;
    }
  }

  return success;
}

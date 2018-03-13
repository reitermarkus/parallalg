#ifndef __MATRIX_H__
#define __MATRIX_H__

typedef float value_t;

typedef value_t* Matrix;

Matrix create_matrix(size_t n, size_t m);

void release_matrix(Matrix m);

void fill_matrices(Matrix a, Matrix b, size_t n);

void print_matrix(Matrix mat, size_t n, size_t m);

#endif

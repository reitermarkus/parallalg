#ifndef __MATRIX_H__
#define __MATRIX_H__

typedef float value_t;

typedef value_t* Matrix;

Matrix create_matrix(int N, int M);

void release_matrix(Matrix m);

void fill_matrices(Matrix A, Matrix B, int N);

void print_matrix(Matrix m, int N, int M);

#endif

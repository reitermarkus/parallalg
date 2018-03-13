kernel void mat_mul(
    global float* res_mat,
    global float* mat_A,
    global float* mat_B,
    const int N
) {
    // global position in X direction
    int col = get_global_id(0);

    // global position in Y direction
    int row = get_global_id(1);

    printf("col: %d, row: %d\n", col, row);

    float res = 0.0f;

    // compute result for one cell
    for(int i = 0; i < N; i++) {
        res += mat_A[row * N + i] * mat_B[i * N + col];
    }

    // printf("f4 = %2.2v4hlf\n", res);
    res_mat[row * N + col] = res;
}

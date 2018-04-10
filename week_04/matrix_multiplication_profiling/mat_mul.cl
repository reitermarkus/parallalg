kernel void mat_mul(
    global float* res_mat,
    global float* mat_a,
    global float* mat_b,
    const int n
) {
    // global position in X direction
    size_t col = get_global_id(0);

    // global position in Y direction
    size_t row = get_global_id(1);

    float res = 0.0f;

    // compute result for one cell
    for(size_t i = 0; i < n; i++) {
        res += mat_a[row * n + i] * mat_b[i * n + col];
    }

    // printf("f4 = %2.2v4hlf\n", res);
    res_mat[row * n + col] = res;
}

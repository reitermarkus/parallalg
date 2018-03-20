kernel void heat_stencil(
    global float* temperature,
    global float* compute,
    const int heat_source_x,
    const int heat_source_y,
    const int n
) {

    size_t col = get_global_id(0);
    size_t row = get_global_id(1);

    if (row == heat_source_x && col == heat_source_y) {
        compute[row * n + col] = temperature[row * n + col];
    } else {
        float t_cur = temperature[row * n + col];

        // get temperatures left/right and up/down
        float tl = ( col !=  0  ) ? temperature[row * n + (col-1)] : t_cur;
        float tr = ( col != n-1 ) ? temperature[row * n + (col+1)] : t_cur;
        float tu = ( row !=  0  ) ? temperature[(row-1) * n + col] : t_cur;
        float td = ( row != n-1 ) ? temperature[(row+1) * n + col] : t_cur;

        // update temperature at current point
        compute[row * n + col] = t_cur + 0.2f * (tl + tr + tu + td + (-4 * t_cur));
    }
}

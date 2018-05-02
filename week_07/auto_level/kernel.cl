kernel void reduce_sum(global const ulong* input_image, local ulong* accumulator, const ulong length, const int components, global ulong* result) {
  for (size_t c = 0; c < components; c++) {
    size_t local_id = get_local_id(0) * components + c;

    if (get_global_id(0) < length) {
      accumulator[local_id] = input_image[get_global_id(0) * components + c];
    } else {
      accumulator[local_id] = 0;
    }
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  for (size_t offset = get_local_size(0) / 2; offset > 0; offset /= 2) {
    for (size_t c = 0; c < components; c++) {
      if (get_local_id(0) < offset) {
        size_t local_id = get_local_id(0) * components + c;
        ulong byte1 = accumulator[local_id];
        ulong byte2 = accumulator[local_id + offset];

        printf("a[%d] = a[%d] + a[%d] = %u + %u\n", local_id, local_id, local_id + offset, byte1, byte2);

        accumulator[local_id] = byte1 + byte2;
      }

      barrier(CLK_LOCAL_MEM_FENCE);
    }
  }

  // Read local results.
  if (get_local_id(0) == 0 && get_local_id(1) == 0) {
    size_t group_id = get_group_id(0);

    for (size_t c = 0; c < components; c++) {
      result[group_id * components + c] = accumulator[c];
    }
  }
}

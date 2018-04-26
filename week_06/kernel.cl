kernel void reduce(global const ulong* bytes, local ulong* accumulator, const ulong length, global ulong* result) {
  size_t local_id = get_local_id(0);
  size_t local_size = get_local_size(0);

  size_t global_id = get_global_id(0);
  size_t global_size = get_global_size(0);

  size_t group_id = get_group_id(0);
  size_t num_groups = get_num_groups(0);

  if (global_id < length) {
    accumulator[local_id] = bytes[global_id];
  } else {
    accumulator[local_id] = 0;
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  for (size_t offset = local_size / 2; offset > 0; offset /= 2) {
    if (local_id < offset) {
      ulong byte1 = accumulator[local_id];
      ulong byte2 = accumulator[local_id + offset];

      // printf("a[%d] = a[%d] + a[%d] = %d + %d\n", local_id, local_id, local_id + offset, byte1, byte2);

      accumulator[local_id] = byte1 + byte2;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // Read local results.
  if (local_id == 0) {
    result[group_id] = accumulator[0];
  }

  barrier(CLK_GLOBAL_MEM_FENCE);

  // Reduce all local results.
  if (global_id == 0) {
    for (size_t g = 1; g < num_groups; g++)  {
      result[0] += result[g];
    }
  }
}

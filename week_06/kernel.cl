kernel void reduce(global const ulong* bytes, local ulong* accumulator, const ulong length, global ulong* result) {
  size_t local_id = get_local_id(0);
  size_t local_size = get_local_size(0);

  size_t global_id = get_global_id(0);
  size_t global_size = get_global_size(0);

  size_t group_id = get_group_id(0);

  size_t start_id = (group_id * local_size + local_id) * 2;

  if (start_id < length) {
    ulong byte1 = bytes[start_id];
    ulong byte2 = start_id + 1 < length ? bytes[start_id + 1] : 0;

    // printf("r[%d] = b[%d] + b[%d] = %d + %d\n", group_id * local_size + local_id, start_id, start_id + 1, byte1, byte2);

    result[group_id * local_size + local_id] = byte1 + byte2;
  }
}

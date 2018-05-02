size_t extend_to_multiple(size_t value, size_t divisor) {
  size_t rest = value % divisor;

  if (rest == 0) {
    return value;
  }

  return value + (divisor - rest);
}

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

  size_t offset = get_local_size(0);

  while (true) {
    offset = extend_to_multiple(offset / components, components);

    for (size_t c = 0; c < components; c++) {
      size_t local_id = get_local_id(0) * components + c;
      size_t offset_id = local_id + offset;
      if (local_id < offset) {
        ulong byte1 = accumulator[local_id];
        ulong byte2 = accumulator[offset_id];

        if (byte2 != 0) {
          printf("offset = %d\n", offset);
          printf("a[%d] = a[%d] + a[%d] = %u + %u\n", local_id, local_id, offset_id, byte1, byte2);
        }

        accumulator[local_id] = byte1 + byte2;
      }


      barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (offset <= components){
      break;
    }
  }

  // Read local results.
  if (get_local_id(0) == 0) {
    size_t group_id = get_group_id(0);

    for (size_t c = 0; c < components; c++) {
      result[group_id * components + c] = accumulator[c];
    }
  }
}

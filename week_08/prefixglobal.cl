kernel void prefix_sum(global const ulong* input, local ulong* temp, global ulong* output, const ulong n) {
  size_t thid = get_local_id(0);
  size_t local_size = get_local_size(0);
  size_t global_id = get_global_id(0);
  size_t pout = 0, pin = 1;

  // load input into local memory
  // Exclusive scan: shift right by one and set first element to 0

  temp[thid] = (global_id > 0) ? input[global_id - 1] : 0;
  barrier(CLK_LOCAL_MEM_FENCE);

  for(int offset = 1; offset < local_size; offset <<= 1) {
    pout = 1 - pout; // swap double buffer indices
    pin = 1 - pout;

    if (global_id < n) {
      if(thid >= offset) {
        temp[pout * local_size + thid] = temp[pin * local_size + thid] + temp[pin * local_size + thid - offset];
      } else {
        temp[pout * local_size + thid] = temp[pin * local_size + thid];
      }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  output[global_id] = temp[pout * local_size + thid]; // write output
}

kernel void update(global const ulong* input, global ulong* output, const ulong n) {
  size_t global_id = get_global_id(0);

  if (global_id >= n) {
    return;
  }

  output[global_id] = input[global_id];

  size_t group_id = get_group_id(0);
  size_t local_size = get_local_size(0);

  for (size_t i = 0; i < group_id; i++) {
    output[global_id] += input[(i + 1) * local_size - 1];
  }
}

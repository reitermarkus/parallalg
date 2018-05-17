kernel void hillis_steele(global const ulong* input, local ulong* temp, global ulong* output, const ulong n) {
  int thid = get_global_id(0);
  int pout = 0, pin = 1;

  // load input into local memory
  // Exclusive scan: shift right by one and set first element to 0
  temp[thid] = (thid > 0) ? input[thid - 1] : 0;
  barrier(CLK_LOCAL_MEM_FENCE);

  for(int offset = 1; offset < n; offset <<= 1) {
    pout = 1 - pout; // swap double buffer indices
    pin = 1 - pout;

    if(thid >= offset) {
      temp[pout * n + thid] = temp[pin * n + thid] + temp[pin * n + thid - offset];
    } else {
      temp[pout * n + thid] = temp[pin * n + thid];
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  output[thid] = temp[pout * n + thid]; // write output
}

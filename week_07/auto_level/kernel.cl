#define INIT_ACCUMULATOR(default_value) do {\
  for (size_t c = 0; c < components; c++) {\
    size_t local_id = get_local_id(0) * components + c;\
    \
    if (get_global_id(0) < length) {\
      accumulator[local_id] = input[get_global_id(0) * components + c];\
    } else {\
      accumulator[local_id] = default_value;\
    }\
  }\
  \
  barrier(CLK_LOCAL_MEM_FENCE);\
} while (0);

#define LOOP(function) do {\
  for (size_t offset = get_local_size(0) / 2; offset > 0; offset /= 2) {\
    for (size_t c = 0; c < components; c++) {\
      size_t local_id = get_local_id(0) * components + c;\
      size_t offset_id = local_id + offset * components;\
      \
      if (get_local_id(0) < offset) {\
        ulong byte1 = accumulator[local_id];\
        ulong byte2 = accumulator[offset_id];\
        accumulator[local_id] = function(byte1, byte2);\
      }\
    }\
    \
    barrier(CLK_LOCAL_MEM_FENCE);\
  }\
} while (0);

#define WRITE_RESULTS do {\
  barrier(CLK_LOCAL_MEM_FENCE);\
  \
  if (get_local_id(0) == 0) {\
    size_t group_id = get_group_id(0);\
    \
    for (size_t c = 0; c < components; c++) {\
      result[group_id * components + c] = accumulator[c];\
    }\
  }\
} while (0);

ulong sum(ulong byte1, ulong byte2) {
  return byte1 + byte2;
}

kernel void reduce_min(global const ulong* input, local ulong* accumulator, const ulong length, const int components, global ulong* result) {
  INIT_ACCUMULATOR(255);
  LOOP(min);
  WRITE_RESULTS;
}

kernel void reduce_max(global const ulong* input, local ulong* accumulator, const ulong length, const int components, global ulong* result) {
  INIT_ACCUMULATOR(0);
  LOOP(max);
  WRITE_RESULTS;
}

kernel void reduce_sum(global const ulong* input, local ulong* accumulator, const ulong length, const int components, global ulong* result) {
  INIT_ACCUMULATOR(0);
  LOOP(sum);
  WRITE_RESULTS;
}

kernel void adjust(global ulong* image, global const float* min_fac, global const float* max_fac, global const uchar* average, const ulong length, const int components) {
  if (get_global_id(0) >= length) {
    return;
  }

  for (size_t c = 0; c < components; c++) {
    size_t index = get_global_id(0) * components + c;

    uchar val = image[index];

    float v = (float)(val - average[c]);

    if (val < average[c]) {
      v *= min_fac[c];
    } else {
      v *= max_fac[c];
    }

    image[index] = (v + average[c]);
  }
}

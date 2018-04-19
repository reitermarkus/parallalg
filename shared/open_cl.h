#pragma once

#include <stdio.h>

#include "opencl.h"

typedef struct kernel_code {
  const char *code;
  size_t size;
} kernel_code;

kernel_code load_code(const char *filename) {
  size_t MAX_SOURCE_SIZE = 0x100000;

  FILE *fp;

  /* Load the source code containing the kernel*/
  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel from file %s\n", filename);
    exit(1);
  }

  kernel_code res;
  res.code = (char *)malloc(MAX_SOURCE_SIZE);
  res.size = fread((char *)res.code, 1, MAX_SOURCE_SIZE, fp);
  fclose(fp);

  return res;
}

void release_code(kernel_code code) { free((char *)code.code); }

cl_platform_id platform_id;
cl_device_id device_id;
cl_command_queue command_queue;
cl_program program;
cl_kernel kernel;
cl_context context;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;
cl_event profiling_event;

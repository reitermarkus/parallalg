#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "cl_utils.h"
#include "matrix.h"
#include "open_cl.h"
#include "utils.h"

void print_array(unsigned long* ary, int len) {
  printf("[");

  for(size_t i = 0; i < len; ++i) {
    printf("%ld", ary[i]);
    if (i != len - 1) { printf(", "); }
  }

  printf("]\n");
}

int main(int argc, char **argv) {
  srand(0);

  const char *program_name = "../prefixglobal.cl";

  unsigned long n = 30;

  // 'parsing' optional input parameter = problem size
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  // init
  unsigned long *array = malloc(sizeof(unsigned long) * n);

  for (size_t i = 0; i < n; i++) {
    array[i] = 1;
  }

  // ---------- compute ----------

  timestamp begin = now();

  cl_int ret;

  cl_context context;
  cl_device_id device_id = cluInitDevice(DEVICE_NUMBER, &context, NULL);
  cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);

  // ------------ Part B (data management) ------------ //
  size_t vec_size = sizeof(unsigned long) * n;
  cl_mem bytes = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for bytes");
  cl_mem result = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for result");

  ret = clEnqueueWriteBuffer(command_queue, bytes, CL_TRUE, 0, vec_size, array, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write bytes to device");

  cl_program program = cluBuildProgramFromFile(context, device_id, program_name, NULL);

  // 11) schedule kernel
  size_t global_work_offset = 0;
  size_t local_work_size = 8;
  size_t global_work_size = extend_to_multiple(n, local_work_size);

  cl_kernel prefix_sum_kernel = clCreateKernel(program, "prefix_sum", &ret);
  CLU_ERRCHECK(ret, "Failed to create hillis and steele kernel.");

  cluSetKernelArguments(prefix_sum_kernel, 4,
    sizeof(cl_mem), (void *)&bytes,
    sizeof(unsigned long) * (local_work_size * 2), NULL,
    sizeof(cl_mem), (void *)&result,
    sizeof(unsigned long), &n
  );

  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, prefix_sum_kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, NULL), "Failed to enqueue 1D kernel");

  cl_kernel update_kernel = clCreateKernel(program, "update", &ret);
  CLU_ERRCHECK(ret, "Failed to create update kernel.");

  cluSetKernelArguments(update_kernel, 3,
    sizeof(cl_mem), (void *)&result,
    sizeof(cl_mem), (void *)&bytes,
    sizeof(unsigned long), &n
  );

  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, update_kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, NULL), "Failed to enqueue 1D kernel");

  unsigned long* output = malloc(sizeof(unsigned long) * n);
  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, bytes,
    CL_TRUE, 0, sizeof(unsigned long) * n, output, 0, NULL, NULL), "Failed reading back result");

  timestamp end = now();
  printf("Total time: %.3f ms\n", (end - begin) * 1000);

  printf("Input:  ");
  print_array(array, n);
  printf("Output: ");
  print_array(output, n);

  free(array);
  free(output);

  // ---------- cleanup ----------
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(prefix_sum_kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseKernel(update_kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(bytes), "Failed to release Matrix A");
  CLU_ERRCHECK(clReleaseMemObject(result), "Failed to release Matrix B");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context");

  // done
  return EXIT_SUCCESS;
}

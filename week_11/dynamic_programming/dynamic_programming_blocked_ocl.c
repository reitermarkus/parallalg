#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "cl_utils.h"
#include "open_cl.h"
#include "utils.h"

void print_array(unsigned long* ary, int len) {
  printf("[");

  for (size_t i = 0; i < len; i++) {
    printf("%ld", ary[i]);
    if (i != len - 1) { printf(", "); }
  }

  printf("]\n");
}

int main(int argc, char** argv) {
  int min_size = 10;
  int max_size = 20;

  // read problem size
  int n                    = argc >= 2 ? atoi(argv[1]) : 2000;
  unsigned long block_size = argc >= 3 ? atoi(argv[2]) : 22;
  int s = n + 1;

  // init
  srand(0);
  unsigned long* l = malloc(sizeof(unsigned long) * s);
  for (size_t i = 0; i < s; i++) {
    l[i] = ((rand() / (float)RAND_MAX) * (max_size - min_size)) + min_size;
  }

  unsigned long* minimum_costs = malloc(sizeof(*minimum_costs) * n * n);
  // ---------- compute ----------

  timestamp begin = now();

  // initialize solutions for costs of single matrix
  for (size_t i = 0; i < n; i++) {
    minimum_costs[i * n + i] = 0; // there is no multiplication cost for those sub-terms
  }

  unsigned long num_blocks = ceil(n / (double)block_size);

  cl_int ret;

  cl_context context;
  cl_command_queue command_queue;
  cl_device_id device_id = cluInitDeviceWithProperties(DEVICE_NUMBER, &context, &command_queue, CL_QUEUE_PROFILING_ENABLE);

  // ------------ Part B (data management) ------------ //
  size_t vec_size = sizeof(unsigned long) * n;
  cl_mem l_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for l");
  cl_mem minimum_costs_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for minimum costs");

  ret = clEnqueueWriteBuffer(command_queue, minimum_costs_mem, CL_TRUE, 0, vec_size, minimum_costs, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write minimum costs to device");

  free(minimum_costs);
  free(l);

  cl_program program = cluBuildProgramFromFile(context, device_id, "dynamic_programming.cl", NULL);

  // 11) schedule kernel
  size_t global_work_offset = 0;
  size_t local_work_size = 256;
  size_t global_work_size = extend_to_multiple(n, local_work_size);

  cl_kernel kernel = clCreateKernel(program, "dynamic_programming", &ret);
  CLU_ERRCHECK(ret, "Failed to create dynamic_programming kernel from program");

  cluSetKernelArguments(kernel, 3,
    sizeof(cl_mem), (void*)&l_mem,
    sizeof(cl_mem), (void*)&minimum_costs_mem,
    sizeof(unsigned long), &num_blocks
  );

  cl_event profiling_event;
  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");

  // wait until event finishes
  ret = clWaitForEvents(1, &profiling_event);
  // get profiling data
  cl_ulong event_start_time = (cl_ulong)0;
  cl_ulong event_end_time = (cl_ulong)0;
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, NULL);
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, NULL);

  cl_ulong output[n];
  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, minimum_costs_mem,
    CL_TRUE, 0, sizeof(unsigned long) * n, output, 0, NULL, NULL), "Failed reading back result");

  unsigned long kernel_total_time = (unsigned long)(event_end_time - event_start_time);
  printf("Total Kernel Execution Time: %f ms\n", kernel_total_time * 1.0e-6);

  timestamp end = now();
  printf("Total time: %.3f ms\n", (end - begin) * 1000);

  // ---------- cleanup ----------
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(minimum_costs_mem), "Failed to release Matrix A");
  CLU_ERRCHECK(clReleaseMemObject(l_mem), "Failed to release Matrix B");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context");

  // done
  return EXIT_SUCCESS;
}

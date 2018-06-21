#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "cl_utils.h"
#include "open_cl.h"
#include "utils.h"

int main(int argc, char** argv) {
  unsigned long min_size = 10;
  unsigned long max_size = 20;

  // read problem size
  unsigned long n          = argc >= 2 ? atoi(argv[1]) : 2000;
  unsigned long block_size = argc >= 3 ? atoi(argv[2]) : 22;

  printf("Computing minimum cost for multiplying %lu matrices ...\n", n);

  // init
  srand(0);
  unsigned long s = n + 1;
  unsigned long* sizes = NULL;
  unsigned long sizes_size = sizeof(*sizes) * s;
  sizes = malloc(sizes_size);

  for (size_t i = 0; i < s; i++) {
    sizes[i] = ((rand() / (float)RAND_MAX) * (max_size - min_size)) + min_size;
  }

  unsigned long* minimum_costs = NULL;
  unsigned long minimum_costs_size = sizeof(*minimum_costs) * n * n;
  minimum_costs = malloc(minimum_costs_size);

  // ---------- compute ----------

  timestamp begin = now();

  cl_int ret;

  cl_context context;
  cl_command_queue command_queue;
  cl_device_id device_id = cluInitDeviceWithProperties(DEVICE_NUMBER, &context, &command_queue, CL_QUEUE_PROFILING_ENABLE);

  // ------------ Part B (data management) ------------ //
  cl_mem sizes_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizes_size, sizes, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for sizes");

  cl_mem minimum_costs_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, minimum_costs_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for minimum costs");

  ret = clEnqueueWriteBuffer(command_queue, minimum_costs_mem, CL_TRUE, 0, minimum_costs_size, minimum_costs, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write minimum costs to device");

  free(sizes);

  cl_program program = cluBuildProgramFromFile(context, device_id, "dynamic_programming.cl", NULL);

  cl_kernel kernel = clCreateKernel(program, "iterate_cells", &ret);
  CLU_ERRCHECK(ret, "Failed to create iterate_cells kernel from program");

  unsigned long num_blocks = ceil(n / (double)block_size);

  cl_event* event_matrix = malloc(sizeof(cl_event) * num_blocks * num_blocks);

  // iterate through blocks in wave-front order
  for (unsigned long d = 0; d < num_blocks; d++) {
    // Enqueue all blocks in parallel.
    for (unsigned long i = 0; i < num_blocks - d; i++) {
      unsigned long j = i + d;

      cluSetKernelArguments(kernel, 6,
        sizeof(unsigned long), &n,
        sizeof(unsigned long), &block_size,
        sizeof(unsigned long), &i,
        sizeof(unsigned long), &j,
        sizeof(cl_mem),        (void*)&minimum_costs_mem,
        sizeof(cl_mem),        (void*)&sizes_mem
      );

      size_t global_work_offset = 0;
      size_t global_work_size = 1;
      size_t local_work_size = 1;

      cl_event wait_list[] = { event_matrix[(i + 1) * num_blocks + j], event_matrix[i * num_blocks + (j - 1)] };

      CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
        &global_work_offset, &global_work_size, &local_work_size, i <= j ? 0 : 2, i <= j ? NULL : wait_list, &event_matrix[i * num_blocks + j]), "Failed to enqueue 1D kernel");
    }
  }

  cl_event first_event = event_matrix[0 * num_blocks + 0];
  cl_event last_event = event_matrix[0 * num_blocks + num_blocks - 1];

  // wait until event finishes
  CLU_ERRCHECK(clWaitForEvents(1, &last_event), "Failed waiting for last event.");

  // get profiling data
  cl_ulong event_start_time = (cl_ulong)0;
  cl_ulong event_end_time = (cl_ulong)0;
  ret = clGetEventProfilingInfo(first_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, NULL);
  ret = clGetEventProfilingInfo(last_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, NULL);


  free(event_matrix);

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, minimum_costs_mem,
    CL_TRUE, 0, minimum_costs_size, minimum_costs, 0, NULL, NULL), "Failed reading back result");

  printf("Minimal costs: %lu FLOPS\n", minimum_costs[0 * n + n - 1]);

  free(minimum_costs);

  unsigned long kernel_total_time = (unsigned long)(event_end_time - event_start_time);
  printf("Total kernel execution time: %.3fs\n", kernel_total_time * 1.0e-9);

  timestamp end = now();
  printf("Total time: %.3fs\n", end - begin);

  // ---------- cleanup ----------
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(minimum_costs_mem), "Failed to release minimum costs");
  CLU_ERRCHECK(clReleaseMemObject(sizes_mem), "Failed to release sizes");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context");

  // done
  return EXIT_SUCCESS;
}

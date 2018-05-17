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

  cl_kernel kernel = clCreateKernel(program, "prefix_sum", &ret);
  CLU_ERRCHECK(ret, "Failed to create hillis and steele kernel from program");

  cluSetKernelArguments(kernel, 4,
    sizeof(cl_mem), (void *)&bytes,
    sizeof(unsigned long) * (local_work_size * 2), NULL,
    sizeof(cl_mem), (void *)&result,
    sizeof(unsigned long), &n
  );

  cl_event profiling_event;
  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");


  // wait until event finishes
  ret = clWaitForEvents(1, &profiling_event);
  // get profiling data
  cl_ulong event_start_time = (cl_ulong) 0;
  cl_ulong event_end_time = (cl_ulong) 0;
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, NULL);
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, NULL);

  unsigned long* output = malloc(sizeof(unsigned long) * n);
  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, result,
    CL_TRUE, 0, sizeof(unsigned long) * n, output, 0, NULL, NULL), "Failed reading back result");

  unsigned long kernel_total_time = (unsigned long)(event_end_time - event_start_time);
  printf("Total Kernel Execution Time: %f ms\n", kernel_total_time * 1.0e-6);

  timestamp end = now();
  printf("Total time: %.3f ms\n", (end - begin) * 1000);

  printf("Input:  ");
  print_array(array, n);
  printf("Output: ");
  print_array(output, n);

  size_t aux_len = global_work_size / local_work_size;
  unsigned long* aux = malloc(sizeof(unsigned long) * aux_len);

  for(size_t i = 1; i < aux_len; i++) {
    aux[i - 1] = output[(i * local_work_size) - 1];
  }

  aux[aux_len - 1] = output[n - 1];

  printf("Aux: ");
  print_array(aux, aux_len);

  for(size_t i = aux_len - 1; i > 0; i--) {
    aux[i] = aux[i - 1];
  }

  aux[0] = 0;

  for(size_t i = 1; i < aux_len; i++) {
    aux[i] += aux[i - 1];
  }

  printf("Aux Prefix Sum: ");
  print_array(aux, aux_len);

  for(size_t i = 1; i < aux_len; i++) {
    for(size_t j = 0; j < local_work_size; j++) {
      output[i * local_work_size + j] += aux[i];
    }
  }

  printf("Final Output: ");
  print_array(output, n);

  free(array);
  free(aux);
  free(output);

  // ---------- cleanup ----------
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel");
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "cl_utils.h"
#include "matrix.h"
#include "open_cl.h"
#include "utils.h"

static unsigned long n = 1000000;

static size_t vec_size;

static cl_mem bytes;
static cl_mem result;

void print_profiling_info(const char* event_description) {
  cl_ulong event_total_time = (cl_ulong)0;
  size_t ev_return_bytes;

  // wait until event finishes
  ret = clWaitForEvents(1, &profiling_event);
  // get profiling data
  cl_ulong event_start_time = (cl_ulong) 0;
  cl_ulong event_end_time = (cl_ulong) 0;
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, &ev_return_bytes);
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, &ev_return_bytes);

  unsigned long total = (unsigned long)(event_end_time - event_start_time);
  event_total_time += total;
  printf("%s:\n", event_description);
  printf("  %f ms\n", total * 1.0e-6);
}

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

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
}

int main(int argc, char **argv) {
  srand(time(NULL));

  const char *program_name = "../kernel.cl";

  // 'parsing' optional input parameter = problem size
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  // init
  long *array = malloc(sizeof(long) * n);
  long *result_array = malloc(sizeof(long) * n);

  for (long i = 0; i < n; i++) {
    array[i] = (rand() % 2);
  }

  // ---------- compute ----------

  timestamp begin = now();

  device_id = cluInitDevice(DEVICE_NUMBER, &context, &command_queue);
  command_queue = clCreateCommandQueueWithProperties(context, device_id, CL_QUEUE_PROFILING_ENABLE , &ret);

  // ------------ Part B (data management) ------------ //
  vec_size = sizeof(long) * n;
  bytes = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for bytes");
  result = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for result");

  ret = clEnqueueWriteBuffer(command_queue, bytes, CL_TRUE, 0, vec_size, array, 0, NULL, &profiling_event);
  CLU_ERRCHECK(ret, "Failed to write bytes to device");
  print_profiling_info("Write bytes into device memory");


  program = cluBuildProgramFromFile(context, device_id, program_name, NULL);

  // 11) schedule kernel
  size_t global_work_offset[] = {0};
  size_t local_work_size[] = {256};
  size_t global_work_size = extend_to_multiple(n, local_work_size[0]);

  kernel = clCreateKernel(program, "reduce", &ret);
  CLU_ERRCHECK(ret, "Failed to create reduce kernel from program");

  cluSetKernelArguments(kernel, 4,
    sizeof(cl_mem), (void *)&bytes,
    (local_work_size[0] + 2) * sizeof(long), NULL,
    sizeof(n), &n,
    sizeof(cl_mem), (void *)&result
  );


  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
    global_work_offset, &global_work_size, local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");

    print_profiling_info("Run kernel");

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, result,
    CL_TRUE, 0, vec_size, result_array, 0, NULL, &profiling_event), "Failed reading back result");

  print_profiling_info("Read result matrix from device to host");

  timestamp end = now();
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  // ---------- cleanup ----------

  clean_up();

  // done
  return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "matrix.h"
#include "common/helpers.h"
#include "open_cl.h"
#include "cl_utils.h"

static const int dimension = 2;
static unsigned long n = 500;

static size_t vec_size;
static Matrix matrix_a;

static cl_mem dev_vec_a;
static cl_mem dev_vec_b;

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue),    "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue),   "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel),   "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(dev_vec_a), "Failed to release Matrix A");
  CLU_ERRCHECK(clReleaseMemObject(dev_vec_b), "Failed to release Matrix B");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context),            "Failed to release OpenCL context");
}

int main(int argc, char** argv) {
  const char* program_name = "heat_stencil.cl";

  // 'parsing' optional input parameter = problem size
  if (argc > 1) {
    n = atoi(argv[1]);
  }
  int T = n * 100;
  printf("Computing heat-distribution for room size n=%d for T=%d timesteps\n", n, T);

  // create a buffer for storing temperature fields
  matrix_a = create_matrix(n, n);

  // set up initial conditions in matrix_a
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      matrix_a[i * n + j] = 273.0f; // temperature is 0°C everywhere (273K)
    }
  }

  // and there is a heat source in one corner
  unsigned long source_x = n / 4;
  unsigned long source_y = n / 4;
  matrix_a[source_x * n + source_y] = 273.0f + 60.0f;

  printf("Initial:\n");
  print_temperature(matrix_a, n, n);

  // ---------- compute ----------

  timestamp begin = now();

  device_id = cluInitDevice(DEVICE_NUMBER, &context, &command_queue);

  // ------------ Part B (data management) ------------ //
  vec_size = sizeof(value_t) * n * n;
  dev_vec_a = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for vector A");
  dev_vec_b = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for vector B");

  ret = clEnqueueWriteBuffer(command_queue, dev_vec_a, CL_TRUE, 0, vec_size, matrix_a, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write vector A to device");

  program = cluBuildProgramFromFile(context, device_id, program_name, NULL);

  // 11) schedule kernel
  size_t global_work_offset[] = {0, 0};
  size_t local_work_size[] = {16, 16};
  size_t global_work_size[] = {
    extend_to_multiple(n, local_work_size[0]),
    extend_to_multiple(n, local_work_size[1]),
  };

  kernel = clCreateKernel(program, "calc_temp", &ret);
  CLU_ERRCHECK(ret, "Failed to create calc_temp kernel from program");

  ret = clSetKernelArg(kernel, 2, (local_work_size[0] + 2) * (local_work_size[1] + 2) * sizeof(float), NULL);
  CLU_ERRCHECK(ret, "Failed to set kernel argument with index 2");
  ret = clSetKernelArg(kernel, 3, sizeof(n), &n);
  CLU_ERRCHECK(ret, "Failed to set kernel argument with index 3");
  ret = clSetKernelArg(kernel, 4, sizeof(source_x), &source_x);
  CLU_ERRCHECK(ret, "Failed to set kernel argument with index 4");
  ret = clSetKernelArg(kernel, 5, sizeof(source_y), &source_y);
  CLU_ERRCHECK(ret, "Failed to set kernel argument with index 5");

  // for each time step ..
  for (int t = 0; t < T; t++) {
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_vec_a);
    CLU_ERRCHECK(ret, "Failed to set kernel argument with index 0");
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_vec_b);
    CLU_ERRCHECK(ret, "Failed to set kernel argument with index 1");

    // execute kernel on device
    CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, dimension,
      global_work_offset, global_work_size, local_work_size, 0, NULL, NULL), "Failed to enqueue 2D kernel");

    cl_mem dev_vec_h = dev_vec_a;
    dev_vec_a = dev_vec_b;
    dev_vec_b = dev_vec_h;

    // show intermediate step
    if (!(t % 1000)) {
      CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, dev_vec_a,
        CL_TRUE, 0, vec_size, matrix_a, 0, NULL, NULL), "Failed reading back result");

      printf("Step t=%d:\n", t);
      print_temperature(matrix_a, n, n);
    }
  }

  timestamp end = now();
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  double mflops = n * n * 7 * T / 1000000.0 / (double)(end - begin);
  printf("MFLOPS: %.3f\n", mflops);

  // ---------- check ----------

  bool success = true;
  for (long long i = 0; i < n; i++) {
    for (long long j = 0; j < n; j++) {
      value_t temp = matrix_a[i * n + j];
      if (273.0f <= temp && temp <= 273.0f + 60.0f) continue;
      success = false;
      break;
    }
  }

  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ---------- cleanup ----------

  clean_up();
  free(matrix_a);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

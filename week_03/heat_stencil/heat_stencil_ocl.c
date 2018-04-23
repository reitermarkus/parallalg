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

void init_platform() {
  // initialize OpenCL local state variables
  platform_id = NULL;
  device_id = NULL;
  command_queue = NULL;
  program = NULL;
  kernel = NULL;
  context = NULL;

  // ------------ Part A (resource management) ------------ //
  device_id = cluInitDevice(DEVICE_NUMBER, &context, &command_queue);
}

void init_devices() {
  // ------------ Part B (data management) ------------ //
  vec_size = sizeof(value_t) * n * n;
  dev_vec_a = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  dev_vec_b = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);

  ret = clEnqueueWriteBuffer(command_queue, dev_vec_a, CL_TRUE, 0, vec_size, matrix_a, 0, NULL, NULL);
}

void create_program(const char* program_name) {
  kernel_code code = load_code(program_name);
  program = clCreateProgramWithSource(context, 1, &code.code, (const size_t *)&code.size, &ret);

  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

  if (ret != CL_SUCCESS) {
    size_t size = 1 << 20; // 1MB
    char* msg = malloc(size);
    size_t msg_size;

    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, msg, &msg_size);

    printf("Build Error:\n%s", msg);
    exit(1);
  }
}

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

  // wait for completed operations (there should be none)
  ret = clFlush(command_queue);
  ret = clFinish(command_queue);
  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);

  // free device memory
  ret = clReleaseMemObject(dev_vec_a);
  ret = clReleaseMemObject(dev_vec_b);

  // free management resources
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);
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

  init_platform();
  init_devices();
  create_program(program_name);

  // 11) schedule kernel
  size_t global_work_offset[] = {0, 0};
  size_t global_work_size[] = {n, n};

  kernel = clCreateKernel(program, "calc_temp", &ret);
  ret = clSetKernelArg(kernel, 2, sizeof(n), &n);
  ret = clSetKernelArg(kernel, 3, sizeof(source_x), &source_x);
  ret = clSetKernelArg(kernel, 4, sizeof(source_y), &source_y);

  // for each time step ..
  for (int t = 0; t < T; t++) {
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_vec_a);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_vec_b);

    // execute kernel on device
    ret = clEnqueueNDRangeKernel(command_queue, kernel, dimension, global_work_offset, global_work_size, NULL, 0, NULL, NULL);

    cl_mem dev_vec_h = dev_vec_a;
    dev_vec_a = dev_vec_b;
    dev_vec_b = dev_vec_h;

    // show intermediate step
    if (!(t % 1000)) {
      ret = clEnqueueReadBuffer(command_queue, dev_vec_a, CL_TRUE, 0, vec_size, matrix_a, 0, NULL, NULL);
      printf("Step t=%d:\n", t);
      print_temperature(matrix_a, n, n);
    }
  }

  timestamp end = now();
  printf("Total time: %.3fms\n", (end - begin) * 1000);

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

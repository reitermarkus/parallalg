#include <stdio.h>
#include <utils.h>
#include "matrix/matrix.h"
#include "matrix/helpers.h"
#include "open_cl/open_cl.h"

static int dimension = 2;

static size_t vec_size;

static cl_mem dev_vec_a;
static cl_mem dev_vec_b;
static cl_mem dev_vec_res;

void init_platform() {
  // initialize OpenCL local state variables
  platform_id = NULL;
  device_id = NULL;
  command_queue = NULL;
  program = NULL;
  kernel = NULL;
  context = NULL;

  // ------------ Part A (resource management) ------------ //

  ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);

  ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

  cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, 0};
  command_queue = clCreateCommandQueueWithProperties(context, device_id, properties, &ret);
}

void init_devices() {
  // ------------ Part B (data management) ------------ //

  vec_size = sizeof(value_t) * n * n;
  dev_vec_a   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  dev_vec_b   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  dev_vec_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, vec_size, NULL, &ret);

  ret = clEnqueueWriteBuffer(command_queue, dev_vec_a, CL_TRUE, 0, vec_size, mtx_a, 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_b, CL_TRUE, 0, vec_size, mtx_b, 0, NULL, NULL);
}

void run_kernel(const char *kernel_name) {
  kernel = clCreateKernel(program, kernel_name, &ret);
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_vec_res);
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_vec_a);
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_vec_b);
  ret = clSetKernelArg(kernel, 3, sizeof(n), &n);

  // 11) schedule kernel
  size_t global_work_offset[2] = {0, 0};
  size_t global_work_size[2] = {n, n};

  // execute kernel on device
  ret = clEnqueueNDRangeKernel(command_queue, kernel, dimension,
                               global_work_offset, global_work_size, NULL, 0,
                               NULL, NULL);

  // 12) transfere data back to host

  // CL_FALSE: clEnqueueReadBuffer queues a non-blocking read command and
  // returns if CL_TRUE: segmentation fault with compiler optimization flags -O2
  // or -O3
  ret = clEnqueueReadBuffer(command_queue, dev_vec_res, CL_TRUE, 0, vec_size, mtx_res, 0, NULL, NULL);
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
  ret = clReleaseMemObject(dev_vec_res);

  // free management resources
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);
}

void create_program() {
  kernel_code code = load_code("mat_mul.cl");
  program = clCreateProgramWithSource(context, 1, &code.code, (const size_t *)&code.size, &ret);

  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

  if (ret != CL_SUCCESS) {
    size_t size = 1 << 20; // 1MB
    char *msg = malloc(size);
    size_t msg_size;

    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, msg, &msg_size);

    printf("Build Error:\n%s", msg);
    exit(1);
  }
}

int main(int argc, char **argv) {
  const char *program_name = "mat_mul.cl";
  const char *kernel_name = "mat_mul";

  if (argc > 1) {
    n = atoi(argv[1]);
  }

  printf("Matrix Multiplication with n=%d\n", n);

  // -------------------- SETUP -------------------- //
  init_matrices();

  // -------------------- START -------------------- //
  timestamp begin = now();

  init_platform();
  init_devices();
  create_program();
  run_kernel(kernel_name);
  clean_up();

  timestamp end = now();
  // -------------------- END -------------------- //
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  // ------------------- CHECK ------------------- //
  bool success = check();
  printf("Verification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  release_matrix(mtx_a);
  release_matrix(mtx_b);
  release_matrix(mtx_res);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

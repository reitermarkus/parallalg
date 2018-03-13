#include "common/headers.h"
#include "common/utils.h"
#include "matrix/matrix.h"
#include "open_cl/open_cl.h"

static int N = 1000;
static int dimension = 2;

static Matrix mtx_a;
static Matrix mtx_b;
static Matrix mtx_res;

static size_t vec_size;

static cl_mem dev_vec_A;
static cl_mem dev_vec_B;
static cl_mem dev_vec_RES;

void init_matrices() {
  mtx_a = create_matrix(N, N);
  mtx_b = create_matrix(N, N);
  mtx_res = create_matrix(N, N);

  fill_matrices(mtx_a, mtx_b, N);
}

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

  ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id,
                       &ret_num_devices);

  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

  command_queue =
      clCreateCommandQueueWithProperties(context, device_id, 0, &ret);
}

void init_devices() {
  // ------------ Part B (data management) ------------ //

  vec_size = sizeof(value_t) * N * N;
  dev_vec_A = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                             vec_size, NULL, &ret);
  dev_vec_B = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                             vec_size, NULL, &ret);
  dev_vec_RES = clCreateBuffer(
      context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, vec_size, NULL, &ret);

  ret = clEnqueueWriteBuffer(command_queue, dev_vec_A, CL_TRUE, 0, vec_size,
                             &mtx_a[0], 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_B, CL_TRUE, 0, vec_size,
                             &mtx_b[0], 0, NULL, NULL);
}

void run_kernel(const char *kernel_name) {
  kernel = clCreateKernel(program, kernel_name, &ret);
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_vec_RES);
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_vec_A);
  ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_vec_B);
  ret = clSetKernelArg(kernel, 3, sizeof(int), &N);

  // 11) schedule kernel
  size_t global_work_offset[2] = {0, 0};
  size_t global_work_size[2] = {N, N};

  // execute kernel on device
  ret = clEnqueueNDRangeKernel(command_queue, kernel, dimension,
                               global_work_offset, global_work_size, NULL, 0,
                               NULL, NULL);

  // dummy event, for waiting until kernel finish.
  // without waiting for a dummy event this block of code should be in main
  // method, otherwise --> segmentation fault
  cl_event event;
  clWaitForEvents(0, &event);

  // 12) transfere data back to host

  // CL_FALSE: clEnqueueReadBuffer queues a non-blocking read command and
  // returns if CL_TRUE: segmentation fault with compiler optimization flags -O2
  // or -O3
  ret = clEnqueueReadBuffer(command_queue, dev_vec_RES, CL_TRUE, 0, vec_size,
                            &mtx_res[0], 0, NULL, NULL);
}

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

  // wait for completed operations (there should be none)
  ret = clFlush(command_queue);
  ret = clFinish(command_queue);
  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);

  // free device memory
  ret = clReleaseMemObject(dev_vec_A);
  ret = clReleaseMemObject(dev_vec_B);
  ret = clReleaseMemObject(dev_vec_RES);

  // free management resources
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);
}

bool check() {
  bool success = true;
  for (long long i = 0; i < N; i++) {
    for (long long j = 0; j < N; j++) {
      if (mtx_res[i * N + j] == i * j)
        continue;
      success = false;
      break;
    }
  }
  return success;
}

int main(int argc, char **argv) {

  const char *program_name = "mat_mul.cl";
  const char *kernel_name = "mat_mul";

  if (argc > 1) {
    N = atoi(argv[1]);
  }

  printf("Matrix Multiplication with N=%d\n", N);

  // -------------------- SETUP -------------------- //
  init_matrices();

  // -------------------- START -------------------- //
  timestamp begin = now();

  init_platform();
  init_devices();

  // --------------- CREATE PROGRAM ---------------- //
  // if I move this part into a separate function --> segmentation fault

  kernel_code code = load_code(program_name);
  program = clCreateProgramWithSource(context, 1, &code.code,
                                      (const size_t *)&code.size, &ret);

  ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

  if (ret != CL_SUCCESS) {
    size_t size = 1 << 20; // 1MB
    char *msg = malloc(size);
    size_t msg_size;

    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, msg,
                          &msg_size);

    printf("Build Error:\n%s", msg);
    exit(1);
  }

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

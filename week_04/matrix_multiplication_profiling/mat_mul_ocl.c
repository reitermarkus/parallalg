#include <stdio.h>
#include "utils.h"
#include "matrix.h"
#include "open_cl.h"
#include "cl_utils.h"

static int dimension = 2;

static size_t vec_size;

static cl_mem dev_vec_a;
static cl_mem dev_vec_b;
static cl_mem dev_vec_res;

static cl_ulong event_total_time = (cl_ulong)0;
static size_t ev_return_bytes;

void print_profiling_info(const char* event_description) {
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
  CLU_ERRCHECK(clFlush(command_queue),    "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue),   "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel),   "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(dev_vec_a), "Failed to release Matrix A");
  CLU_ERRCHECK(clReleaseMemObject(dev_vec_b), "Failed to release Matrix B");
  CLU_ERRCHECK(clReleaseMemObject(dev_vec_res), "Failed to release Matrix C");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context),            "Failed to release OpenCL context");
}

void create_program() {
  program = cluBuildProgramFromFile(context, device_id, "mat_mul.cl", NULL);

  if (ret != CL_SUCCESS) {
    size_t size = 1 << 20; // 1MB
    char* msg = malloc(size);
    size_t msg_size;

    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, msg, &msg_size);

    printf("Build Error:\n%s", msg);
    exit(1);
  }
}

int main(int argc, char** argv) {
  const char* program_name = "mat_mul.cl";
  const char* kernel_name = "mat_mul";

  int n = 1000;
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  printf("Matrix Multiplication with n=%d\n", n);

  // -------------------- SETUP -------------------- //
  Matrix mtx_a = create_matrix(n, n);
  Matrix mtx_b = create_matrix(n, n);

  fill_matrices(mtx_a, mtx_b, n, n);

  Matrix mtx_res = create_matrix(n, n);

  // -------------------- START -------------------- //
  timestamp begin = now();

  // initialize OpenCL local state variables
  platform_id = NULL;
  device_id = NULL;
  command_queue = NULL;
  program = NULL;
  kernel = NULL;
  context = NULL;

  // ------------ Part A (resource management) ------------ //
  device_id = cluInitDevice(0, &context, &command_queue);
  command_queue = clCreateCommandQueueWithProperties(context, device_id, CL_QUEUE_PROFILING_ENABLE , &ret);

  // ------------ Part B (data management) ------------ //

  vec_size = sizeof(value_t) * n * n;
  dev_vec_a   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for matrix A");
  dev_vec_b   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for matrix B");
  dev_vec_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for matrix C");

  ret = clEnqueueWriteBuffer(command_queue, dev_vec_a, CL_TRUE, 0, vec_size, mtx_a, 0, NULL, &profiling_event);
  CLU_ERRCHECK(ret, "Failed to write matrix A to device");
  print_profiling_info("Write matrix A into device memory");
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_b, CL_TRUE, 0, vec_size, mtx_b, 0, NULL, &profiling_event);
  CLU_ERRCHECK(ret, "Failed to write matrix B to device");
  print_profiling_info("Write matrix B into device memory");

  create_program();

  kernel = clCreateKernel(program, kernel_name, &ret);
  CLU_ERRCHECK(ret, "Failed to create mat_mul kernel from program");

  cluSetKernelArguments(kernel, 4,
    sizeof(cl_mem), (void *)&dev_vec_res,
    sizeof(cl_mem), (void *)&dev_vec_a,
    sizeof(cl_mem), (void *)&dev_vec_b,
    sizeof(n), &n
  );

  // 11) schedule kernel
  size_t global_work_offset[2] = {0, 0};
  size_t global_work_size[2] = {n, n};

  // execute kernel on device
  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, dimension,
    global_work_offset, global_work_size, NULL, 0, NULL, &profiling_event), "Failed to enqueue 2D kernel");

  print_profiling_info("Run kernel");

  // 12) transfere data back to host
  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, dev_vec_res,
    CL_TRUE, 0, vec_size, mtx_res, 0, NULL, &profiling_event), "Failed reading back result");

  print_profiling_info("Read result matrix from device to host");

  clean_up();

  timestamp end = now();
  // -------------------- END -------------------- //
  printf("\nGPU total time measure: %f ms\n", event_total_time * 1.0e-06);
  printf("CPU total time measure: %.3f ms\n", (end - begin) * 1000);

  double mflops = n / 1000000.0 * n * n * 2.0 / (double)(end - begin);
  printf("MFLOPS: %.3f\n", mflops);

  // ------------------- CHECK ------------------- //
  bool success = check(mtx_res, n, n);
  printf("\nVerification: %s\n", (success) ? "OK" : "FAILED");

  // ----------------- CLEAN UP ------------------ //
  free(mtx_a);
  free(mtx_b);
  free(mtx_res);

  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include <stdio.h>
#include <stdlib.h>

#include <utils.h>

#include "common/helpers.h"
#include "open_cl/open_cl.h"

static const int dimension = 2;
static int n = 500;

static size_t vec_size;
static Matrix mtx_temperature;
static Matrix mtx_compute;

static cl_mem dev_vec_temperature;
static cl_mem dev_vec_compute;

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
  command_queue = clCreateCommandQueueWithProperties(context, device_id, 0, &ret);
}

void init_devices() {
  // ------------ Part B (data management) ------------ //
  vec_size = sizeof(value_t) * n * n;
  dev_vec_temperature = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  dev_vec_compute = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_temperature, CL_TRUE, 0, vec_size, &mtx_temperature[0], 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_compute, CL_TRUE, 0, vec_size, &mtx_compute[0], 0, NULL, NULL);
}

void create_program(const char* program_name) {
  kernel_code code = load_code(program_name);
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

void run_kernel(const char *kernel_name, int x, int y) {
  kernel = clCreateKernel(program, kernel_name, &ret);
  ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_vec_temperature);
  ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_vec_compute);
  ret = clSetKernelArg(kernel, 2, sizeof(x), &x);
  ret = clSetKernelArg(kernel, 3, sizeof(y), &y);
  ret = clSetKernelArg(kernel, 4, sizeof(n), &n);

  // 11) schedule kernel
  size_t global_work_offset[2] = {0, 0};
  size_t global_work_size[2] = {n, n};

  // execute kernel on device
  ret = clEnqueueNDRangeKernel(command_queue, kernel, dimension, global_work_offset, global_work_size, NULL, 0, NULL, NULL);
  ret = clEnqueueReadBuffer(command_queue, dev_vec_compute, CL_TRUE, 0, vec_size, (void *)mtx_compute, 0, NULL, NULL);
  ret = clEnqueueReadBuffer(command_queue, dev_vec_temperature, CL_TRUE, 0, vec_size, (void *)mtx_temperature, 0, NULL, NULL);
}

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

  // wait for completed operations (there should be none)
  ret = clFlush(command_queue);
  ret = clFinish(command_queue);
  ret = clReleaseKernel(kernel);
  ret = clReleaseProgram(program);
  
  // free device memory
  ret = clReleaseMemObject(dev_vec_temperature);
  ret = clReleaseMemObject(dev_vec_compute);

  // free management resources
  ret = clReleaseCommandQueue(command_queue);
  ret = clReleaseContext(context);
}

void print(Matrix mtx) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%f ", mtx[i * n + j]);
        }
        puts("");
    }
}

int main(int argc, char** argv) {

    const char *program_name = "heat_stencil.cl";
    const char *kernel_name = "heat_stencil";

    // 'parsing' optional input parameter = problem size
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    int T = n * 100;
    printf("Computing heat-distribution for room size n=%d for T=%d timesteps\n", n, T);

    // create a buffer for storing temperature fields
    mtx_temperature = create_matrix(n,n);

    // set up initial conditions in mtx_temperature
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            mtx_temperature[i * n + j] = 273.0f;             // temperature is 0°C everywhere (273K)
        }
    }

    // and there is a heat source in one corner
    int source_x = n / 4;
    int source_y = n / 4;
    mtx_temperature[source_x * n + source_y] = 273.0f + 60.0f;

    printf("Initial:\n");
    print_temperature(mtx_temperature, n, n);

    // ---------- compute ----------

    // create a second buffer for the computation
    mtx_compute = create_matrix(n, n);

    timestamp begin = now();

    init_platform();
    init_devices();
    create_program(program_name);

    // for each time step ..
    for(int t=0; t<T; t++) {

        run_kernel(kernel_name, source_x, source_y);

        // swap matrixes (just pointers, not content)
        Matrix tmp = mtx_temperature;
        mtx_temperature = mtx_compute;
        mtx_compute = tmp;

        // show intermediate step
        if (!(t%1000)) {
            printf("Step t=%d:\n", t);
            print_temperature(mtx_temperature,n,n);
        }
    }


    timestamp end = now();
    printf("Total time: %.3fms\n", (end - begin)*1000);

    // ---------- check ----------

    printf("Final:\n");
    print_temperature(mtx_temperature, n, n);

    bool success = true;
    for(long long i = 0; i<n; i++) {
        for(long long j = 0; j<n; j++) {
            value_t temp = mtx_temperature[i*n+j];
            if (273.0f <= temp && temp <= 273.0f + 60.0f) continue;
            success = false;
            break;
        }
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");

    // ---------- cleanup ----------

    clean_up();
    release_matrix(mtx_compute);
    release_matrix(mtx_temperature);

    // done
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

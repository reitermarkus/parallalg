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
  dev_vec_temperature = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_WRITE_ONLY, vec_size, NULL, &ret);
  dev_vec_compute = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, vec_size, NULL, &ret);
  
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_temperature, CL_TRUE, 0, vec_size, &mtx_temperature[0], 0, NULL, NULL);
  ret = clEnqueueWriteBuffer(command_queue, dev_vec_compute, CL_TRUE, 0, vec_size, &mtx_compute[0], 0, NULL, NULL);
}

void run_kernel(const char *kernel_name) {
  kernel = clCreateKernel(program, kernel_name, &ret);
  // TODO: set args

  // 11) schedule kernel
  size_t global_work_offset[2] = {0, 0};
  size_t global_work_size[2] = {n, n};

  // execute kernel on device
  ret = clEnqueueNDRangeKernel(command_queue, kernel, dimension, global_work_offset, global_work_size, NULL, 0, NULL, NULL);

  ret = clEnqueueReadBuffer(command_queue, dev_vec_compute, CL_TRUE, 0, vec_size, &mtx_compute[0], 0, NULL, NULL);
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

int main(int argc, char** argv) {

    // 'parsing' optional input parameter = problem size
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    int T = n*100;
    printf("Computing heat-distribution for room size n=%d for T=%d timesteps\n", n, T);


    // ---------- setup ----------

    // create a buffer for storing temperature fields
    mtx_temperature = create_matrix(n,n);

    // set up initial conditions in mtx_temperature
    for(int i = 0; i<n; i++) {
        for(int j = 0; j<n; j++) {
            mtx_temperature[i*n+j] = 273;             // temperature is 0°C everywhere (273K)
        }
    }

    // and there is a heat source in one corner
    int source_x = n/4;
    int source_y = n/4;
    mtx_temperature[source_x*n+source_y] = 273 + 60;

    printf("Initial:\n");
    print_temperature(mtx_temperature,n,n);

    // ---------- compute ----------

    // create a second buffer for the computation
    mtx_compute = create_matrix(n,n);

    timestamp begin = now();

    // -- BEGIN ASSIGNMENT --

    // TODO: parallelize the following computation using OpenCL


    // for each time step ..
    for(int t=0; t<T; t++) {

        // .. we propagate the temperature
        for(long long i = 0; i<n; i++) {
            for(long long j = 0; j<n; j++) {

                // center stays constant (the heat is still on)
                if (i == source_x && j == source_y) {
                    mtx_compute[i*n+j] = mtx_temperature[i*n+j];
                    continue;
                }

                // get current temperature at (i,j)
                value_t tc = mtx_temperature[i*n+j];

                // get temperatures left/right and up/down
                value_t tl = ( j !=  0  ) ? mtx_temperature[i*n+(j-1)] : tc;
                value_t tr = ( j != n-1 ) ? mtx_temperature[i*n+(j+1)] : tc;
                value_t tu = ( i !=  0  ) ? mtx_temperature[(i-1)*n+j] : tc;
                value_t td = ( i != n-1 ) ? mtx_temperature[(i+1)*n+j] : tc;

                // update temperature at current point
                mtx_compute[i*n+j] = tc + 0.2 * (tl + tr + tu + td + (-4*tc));
            }
        }

        // swap matrixes (just pointers, not content)
        Matrix H = mtx_temperature;
        mtx_temperature = mtx_compute;
        mtx_compute = H;

        // show intermediate step
        if (!(t%1000)) {
            printf("Step t=%d:\n", t);
            print_temperature(mtx_temperature,n,n);
        }
    }


    // -- END ASSIGNMENT --


    timestamp end = now();
    printf("Total time: %.3fms\n", (end-begin)*1000);

    release_matrix(mtx_compute);


    // ---------- check ----------

    printf("Final:\n");
    print_temperature(mtx_temperature,n,n);

    bool success = true;
    for(long long i = 0; i<n; i++) {
        for(long long j = 0; j<n; j++) {
            value_t temp = mtx_temperature[i*n+j];
            if (273 <= temp && temp <= 273+60) continue;
            success = false;
            break;
        }
    }

    printf("Verification: %s\n", (success)?"OK":"FAILED");

    // ---------- cleanup ----------

    release_matrix(mtx_temperature);

    // done
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

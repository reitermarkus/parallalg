#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "cl_utils.h"
#include "utils.h"

typedef struct {
  cl_device_id device_id;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  size_t max_work_group_size;
} cl_mm_environment;

cl_mm_environment create_mm_environment() {
  cl_mm_environment res;

  // ocl initialization
  res.device_id = cluInitDeviceWithProperties(0, &res.context, &res.queue, CL_QUEUE_PROFILING_ENABLE);

  CLU_ERRCHECK(
    clGetDeviceInfo(res.device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(res.max_work_group_size), &res.max_work_group_size, NULL),
    "Failed to get max work group size."
  );

  // create kernel from source
  cl_int err;
  res.program = cluBuildProgramFromFile(res.context, res.device_id, "mat_mul.cl", "-I ../../shared");
  res.kernel = clCreateKernel(res.program, "mat_mul", &err);
  CLU_ERRCHECK(err, "Failed to create mat_mul kernel from program");

  // done
  return res;
}

void free_mm_environment(cl_mm_environment env) {
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(env.queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(env.queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(env.kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(env.program), "Failed to release program");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(env.queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(env.context), "Failed to release OpenCL context");
}

int main(int argc, char **argv) {
  cl_mm_environment env = create_mm_environment();

  size_t SIZES[] = {500, 734, 1024, 1493, 2345, 4001};
  size_t NUM_SIZES = 6;
  size_t NUM_REPETITION = 3;

  // ------ benchmarking -------

  srand(0);
  printf("Start benchmarking ...\n");

  // the best performance
  double mflops[NUM_SIZES];
  bool all_valid = true;

  // for each size ...
  for (size_t i = 0; i < NUM_SIZES; i++) {
    size_t m = SIZES[i];
    size_t k = SIZES[i];
    size_t n = SIZES[i];
    mflops[i] = 0;

    printf("\nSetting up n=%zu ..\n", n);

    // create input
    restrict Matrix mat_a = create_matrix(m, k);
    restrict Matrix mat_b = create_matrix(k, n);
    restrict Matrix mat_c = create_matrix(m, n);
    restrict Matrix mat_r = create_matrix(m, n);

    // fill matrix
    for (int j = 0; j < k; j++) {
      for (int i = 0; i < m; i++) {
        mat_a[i * k + j] = rand() / (float)RAND_MAX + 0.5; // some matrix
      }

      for (int i = 0; i < n; i++) {
        mat_b[i * k + j] = rand() / (float)RAND_MAX + 0.5; // some other matrix
      }
    }

    // compute reference results
    double cpu_start = now();

    #pragma omp parallel for
    for (int mi = 0; mi < m; mi++) {
      for (int ni = 0; ni < n; ni++) {
        for (int ki = 0; ki < k; ki++) {
          mat_r[mi * n + ni] += mat_a[mi * k + ki] * mat_b[ki * n + ni];
        }
      }
    }

    double cpu_end = now();
    double cpu_duration = cpu_end - cpu_start;
    printf("  CPU setup took %2.3fs / %5.3f GFLOPS\n", cpu_duration, (2.0 * m * k * n) / cpu_duration / 1e9);

    // repeat X times ..
    for (int r = 0; r < NUM_REPETITION; r++) {
      // clear result
      memset(mat_c, 0, sizeof(value_t) * m * n);

      // create buffer on device
      cl_int err;
      cl_mem device_mat_a = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, m * k * sizeof(value_t), NULL, &err);
      CLU_ERRCHECK(err, "Failed to create buffer for matrix A");
      cl_mem device_mat_b = clCreateBuffer(env.context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, k * n * sizeof(value_t), NULL, &err);
      CLU_ERRCHECK(err, "Failed to create buffer for matrix B");
      cl_mem device_mat_c = clCreateBuffer(env.context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, m * n * sizeof(value_t), NULL, &err);

      // transfere data
      err = clEnqueueWriteBuffer(env.queue, device_mat_a, CL_TRUE, 0, m * k * sizeof(value_t), mat_a, 0, NULL, NULL);
      CLU_ERRCHECK(err, "Failed to write matrix A to device");
      err = clEnqueueWriteBuffer(env.queue, device_mat_b, CL_TRUE, 0, k * n * sizeof(value_t), mat_b, 0, NULL, NULL);
      CLU_ERRCHECK(err, "Failed to write matrix B to device");

      // --- perform benchmark ---

      // -- run computation --

      // set arguments and execute kernel
      size_t local_work_size[] = {sqrt(env.max_work_group_size), sqrt(env.max_work_group_size)};
      size_t global_work_size[] = {extend_to_multiple(m, local_work_size[0]), extend_to_multiple(n, local_work_size[1])};

      cluSetKernelArguments(env.kernel, 8,
        sizeof(cl_mem), (void *)&device_mat_a,
        sizeof(cl_mem), (void *)&device_mat_b,
        sizeof(cl_mem), (void *)&device_mat_c,
        local_work_size[0] * local_work_size[1] * sizeof(float), NULL,
        local_work_size[0] * local_work_size[1] * sizeof(float), NULL,
        sizeof(unsigned long), &m,
        sizeof(unsigned long), &k,
        sizeof(unsigned long), &n
      );

      // submit kernel
      cl_event event;
      CLU_ERRCHECK(clEnqueueNDRangeKernel(env.queue, env.kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event), "Failed to enqueue 2D kernel");

      // wait for kernel
      clWaitForEvents(1, &event);

      // test whether kernel finished successfully
      cl_int status;
      clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &status, NULL);
      if (status < 0) {
        CLU_ERRCHECK(-status, "Kernel failed to execute succesfully.");
        exit(1);
      }

      // get execution time
      cl_ulong start, end, duration;
      clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
      clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
      duration = end - start;

      // release event
      CLU_ERRCHECK(clReleaseEvent(event), "Failed to release event");

      // copy results back to host
      err = clEnqueueReadBuffer(env.queue, device_mat_c, CL_TRUE, 0, m * n * sizeof(value_t), mat_c, 0, NULL, NULL);
      CLU_ERRCHECK(err, "Failed reading back result");

      // check result
      bool success = true;
      for (int mi = 0; mi < m; mi++) {
        for (int ni = 0; ni < n; ni++) {
          // if result is close enough, we are fine
          if (fabsf(mat_c[mi * n + ni] - mat_r[mi * n + ni]) < 1e-10) continue;
          // printf("Wrong result for (%d,%d): %f vs. %f\n", i, j, mat_c[i * N + j], mat_r[i * N + j]);
          success = false;
        }
      }

      double seconds = duration / 1e9;
      double current_mflops = (2.0 * m * k * n) / seconds / 1e9;
      printf("  Duration: %2.3fs, GFLOPS: %5.3f, Verification: %s\n", seconds, current_mflops, (success) ? "OK" : "FAILED");

      // keep track of overall success
      if (!success) all_valid = false;

      // record best performance
      if (mflops[i] < current_mflops) mflops[i] = current_mflops;

      // free device memory
      CLU_ERRCHECK(clReleaseMemObject(device_mat_a), "Failed to release Matrix A");
      CLU_ERRCHECK(clReleaseMemObject(device_mat_b), "Failed to release Matrix B");
      CLU_ERRCHECK(clReleaseMemObject(device_mat_c), "Failed to release Matrix C");
    }

    printf("  Performance result for n=%zu: %5.3f\n", n, mflops[i]);

    // --- cleanup ---

    // free host memory
    free(mat_a);
    free(mat_b);
    free(mat_c);
    free(mat_r);
  }

  // cleanup
  free_mm_environment(env);

  // finally: report overall result
  printf("\n");
  printf("-------------------------------------------------\n");

  if (!all_valid) {
    printf("Invalid results encountered, failed!\n");
  } else {
    // overall score: geometric mean of individual best
    double prod = 1;

    for (int i = 0; i < NUM_SIZES; i++) {
      prod *= mflops[i];
    }

    double score = pow(prod, 1.0 / NUM_SIZES);
    printf("Overall result: %5.3f GFLOPS\n", score);
  }
  printf("-------------------------------------------------\n");

  // done
  return EXIT_SUCCESS;
}

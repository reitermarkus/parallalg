#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "cl_utils.h"
#include "common/helpers.h"
#include "matrix.h"
#include "open_cl.h"
#include "utils.h"

static const int dimension = 2;
static unsigned long n = 500;

static size_t vec_size;

void clean_up() {
  // ------------ Part D (cleanup) ------------ //

  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue),
               "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue),
               "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context");
}

int main(int argc, char **argv) {
  srand(time(NULL));

  const char *program_name = "parallel_reduction.cl";

  // 'parsing' optional input parameter = problem size
  if (argc > 1) {
    n = atoi(argv[1]);
  }

  // init
  long *array = (long *)malloc(sizeof(long) * n);

  for (long i = 0; i < n; i++) {
    array[i] = (rand() % 2);
  }

  // ---------- compute ----------

  timestamp begin = now();

  device_id = cluInitDevice(DEVICE_NUMBER, &context, &command_queue);

  // ------------ Part B (data management) ------------ //
  vec_size = sizeof(value_t) * n * n;
  program = cluBuildProgramFromFile(context, device_id, program_name, NULL);

  // 11) schedule kernel
  size_t global_work_offset[] = {0, 0};
  size_t local_work_size[] = {16, 16};
  size_t global_work_size[] = {
      extend_to_multiple(n, local_work_size[0]),
      extend_to_multiple(n, local_work_size[1]),
  };

  kernel = clCreateKernel(program, "parallel_reduction", &ret);
  CLU_ERRCHECK(ret, "Failed to create parallel_reduction kernel from program");

  timestamp end = now();
  printf("Total time: %.3fms\n", (end - begin) * 1000);

  // ---------- cleanup ----------

  clean_up();

  // done
  return 0;
}

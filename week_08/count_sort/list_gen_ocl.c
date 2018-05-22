#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cl_utils.h"
#include "matrix.h"
#include "open_cl.h"
#include "utils.h"
#include "people.h"
#include "tokenize.h"

void print_list(person_t *list, int size) {
  for (int i = 0; i < size; i++) {
    printf("[%d]\tName: %s\tAge: %d\n", i + 1, list[i].name, list[i].age);
  }
}

void create_person_list(person_t *persons, int n) {
  person_t p;

  for (int i = 0; i < n; i++) {
    p.age = rand() % MAX_AGE;
    strcpy(p.name, gen_name());
    persons[i] = p;
  }
}

int main(int argc, char **argv) {
  int size = 10;
  int seed = 1;
  const char *program_name = "../count.cl";

  if (argc > 2) {
    size = atoi(argv[1]);
    seed = atoi(argv[2]);
  }

  srand(seed);
  printf("Generating list of size %d with seed %d\n\n", size, seed);

  person_t *list = (person_t *)malloc(size * sizeof(person_t));
  create_person_list(list, size);

  // -------------- COMPUTE -------------- // 
  // timestamp begin = now();
  cl_int ret;
  cl_context context;
  cl_device_id device_id = cluInitDevice(DEVICE_NUMBER, &context, NULL);
  cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);

  size_t vec_size = sizeof(person_t) * size;
  cl_mem list_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, vec_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for list_buffer");

  ret = clEnqueueWriteBuffer(command_queue, list_buffer, CL_TRUE, 0, vec_size, list, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write list_buffer to device");


  // ------------ STEP 0) find highest number ------------ //
  int max = 0;
  for (int i = 0; i < size; i++) {
    if (list[i].age > max)
      max = list[i].age;
  }
  max++;

  // initialize count array of size max with 0's
  int *count_array = (int *)calloc(max, sizeof(int));

  cl_mem count_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * max, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for count_mem");
  ret = clEnqueueWriteBuffer(command_queue, count_mem, CL_TRUE, 0, vec_size, count_array, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write count_array to device");


  // ------------- STEP 1) count occurences ------------- //
  cl_program program = cluBuildProgramFromFile(context, device_id, program_name, NULL);
  size_t global_work_offset = 0;
  size_t local_work_size = 128;
  size_t global_work_size = extend_to_multiple(size, local_work_size);

  cl_kernel kernel_count = clCreateKernel(program, "count", &ret);
  CLU_ERRCHECK(ret, "Failed to create count kernel from program");

  cluSetKernelArguments(kernel_count, 3,
    sizeof(cl_mem), (void *)&list_buffer,
    sizeof(cl_mem), (void *)&count_mem,
    sizeof(int), &size
  );

  cl_event profiling_event;
  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel_count, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");


  // wait until event finishes
  ret = clWaitForEvents(1, &profiling_event);
  // get profiling data
  cl_ulong event_start_time = (cl_ulong) 0;
  cl_ulong event_end_time = (cl_ulong) 0;
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, NULL);
  ret = clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, NULL);

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, count_mem,
    CL_TRUE, 0, sizeof(int) * max, count_array, 0, NULL, NULL), "Failed reading back result");


  clWaitForEvents(1, &profiling_event);

  for (int i = 0; i < max; i++) {
    printf("%d ", count_array[i]);
  }

  return EXIT_SUCCESS;
}
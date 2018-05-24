#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cl_utils.h"
#include "matrix.h"
#include "open_cl.h"
#include "people.h"
#include "tokenize.h"
#include "utils.h"

void print_list(person_t* list, int size) {
  for (int i = 0; i < size; i++) {
    printf("[%d]\tName: %s\tAge: %d\n", i + 1, list[i].name, list[i].age);
  }
}

void create_person_list(person_t* persons, int n) {
  person_t p;

  for (int i = 0; i < n; i++) {
    p.age = rand() % MAX_AGE;
    strcpy(p.name, gen_name());
    persons[i] = p;
  }
}

void count_sort(person_t* list, int size) {
  int max = 0;

  // step 1) find highest number
  for (int i = 0; i < size; i++) {
    if (list[i].age > max)
      max = list[i].age;
  }

  max++;

  // initialize count array of size max with 0's
  int* count_arr = calloc(max, sizeof(int));

  // step 2) count occurences
  for (int i = 0; i < size; i++) {
    count_arr[list[i].age]++;
  }

  // step 3) prefix sum
  for (int i = 1; i < max; i++) {
    count_arr[i] = count_arr[i] + count_arr[i - 1];
  }

  // initialize a result array
  person_t* result = calloc(size, sizeof(person_t));

  // step 4) insert elements in right order into result array
  for (int i = size - 1; i >= 0; i--) {
    person_t p = list[i];
    result[--count_arr[p.age]] = p;
  }

  memcpy(list, result, sizeof(person_t) * size);
  free(result);
  free(count_arr);
}

unsigned long update_kernel_time(cl_event profiling_event) {
  // wait until event finishes
  clWaitForEvents(1, &profiling_event);
  // get profiling data
  cl_ulong event_start_time = (cl_ulong)0;
  cl_ulong event_end_time = (cl_ulong)0;
  clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &event_start_time, NULL);
  clGetEventProfilingInfo(profiling_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &event_end_time, NULL);

  return (unsigned long)(event_end_time - event_start_time);
}

int main(int argc, char** argv) {
  unsigned long size = 10;
  int seed = 1;
  const char* program_name = "../count_sort.cl";

  if (argc > 1) {
    size = atoi(argv[1]);
  }

  if (argc > 2) {
    seed = atoi(argv[2]);
  }

  srand(seed);
  printf("Generating list of size %ld with seed %d\n\n", size, seed);

  person_t* list = malloc(size * sizeof(person_t));
  create_person_list(list, size);

  // ---------------------- SEQUENTIAL ---------------------- //
  timestamp begin = now();
  count_sort(list, size);
  printf("Sequential sort time:\t%.3f ms\n", (now() - begin) * 1000);

  // ----------------------- PARALLEL ----------------------- //
  cl_int ret;
  cl_context context;
  cl_device_id device_id = cluInitDevice(DEVICE_NUMBER, &context, NULL);
  cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);

  size_t list_size = sizeof(person_t) * size;
  cl_mem list_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, list_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for list_buffer");

  ret = clEnqueueWriteBuffer(command_queue, list_buffer, CL_TRUE, 0, list_size, list, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write list_buffer to device");

  cl_event profiling_event;
  cl_ulong kernel_total_time = (cl_ulong)0;

  // ------------------- STEP 0) find highest number ------------------- //
  begin = now();
  int max = 0;
  for (int i = 0; i < size; i++) {
    if (list[i].age > max)
      max = list[i].age;
  }

  max++;
  unsigned long time_of_max = now() - begin;

  // initialize count array of size max with 0's
  unsigned long* count_array = calloc(max, sizeof( unsigned long));

  cl_mem count_array_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof( unsigned long) * max, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for count_array_mem");
  ret = clEnqueueWriteBuffer(command_queue, count_array_mem, CL_TRUE, 0, sizeof( unsigned long) * max, count_array, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write count_array_mem to device");

  // -------------------- STEP 1) count occurences -------------------- //
  cl_program program = cluBuildProgramFromFile(context, device_id, program_name, NULL);
  size_t global_work_offset = 0;
  size_t local_work_size = 128;
  size_t global_work_size = extend_to_multiple(size, local_work_size);

  cl_kernel count_kernel = clCreateKernel(program, "count", &ret);
  CLU_ERRCHECK(ret, "Failed to create count_kernel kernel from program");

  cluSetKernelArguments(count_kernel, 3,
    sizeof(cl_mem), (void *)&list_buffer,
    sizeof(cl_mem), (void *)&count_array_mem,
    sizeof(unsigned long), &size
  );

  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, count_kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");
  kernel_total_time += update_kernel_time(profiling_event);

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, count_array_mem,
    CL_TRUE, 0, sizeof(int) * max, count_array, 0, NULL, NULL), "Failed reading back result");

  // ----------------------- STEP 2) prefix sum ----------------------- //
  for (int i = 1; i < max; i++) {
    count_array[i] = count_array[i] + count_array[i - 1];
  }

  // ------------------ STEP 3) insert in right order ----------------- //
  cl_mem result_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, list_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for result_mem");

  cl_kernel insert_kernel = clCreateKernel(program, "insert", &ret);
  CLU_ERRCHECK(ret, "Failed to create insert_kernel kernel from program");

  cluSetKernelArguments(insert_kernel, 4,
    sizeof(cl_mem), (void *)&list_buffer,
    sizeof(cl_mem), (void *)&count_array_mem,
    sizeof(cl_mem), (void *)&result_mem,
    sizeof(unsigned long), &size
  );

  ret = clEnqueueWriteBuffer(command_queue, count_array_mem, CL_TRUE, 0, sizeof(unsigned long) * max, count_array, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write count_array_mem to device");

  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, insert_kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, &profiling_event), "Failed to enqueue 1D kernel");
  kernel_total_time += update_kernel_time(profiling_event);

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, result_mem,
    CL_TRUE, 0, list_size, list, 0, NULL, NULL), "Failed reading back result");

  kernel_total_time += time_of_max;
  printf("Parallel sort time:\t%f ms\n", (unsigned long) kernel_total_time * 1.0e-6);

  // ------------------- CLEAN UP ------------------- //
  // wait for completed operations (there should be none)
  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion");
  CLU_ERRCHECK(clReleaseKernel(count_kernel), "Failed to release count_kernel");
  CLU_ERRCHECK(clReleaseKernel(insert_kernel), "Failed to release insert_kernel");
  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program");

  // free device memory
  CLU_ERRCHECK(clReleaseMemObject(count_array_mem), "Failed to release count_array_mem");
  CLU_ERRCHECK(clReleaseMemObject(result_mem), "Failed to release result_mem");
  CLU_ERRCHECK(clReleaseMemObject(list_buffer), "Failed to release result_mem");

  // free management resources
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context");

  free(count_array);
  free(list);

  return EXIT_SUCCESS;
}

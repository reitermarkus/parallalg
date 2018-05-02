#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "utils.h"
#include "open_cl.h"
#include "cl_utils.h"

void reduce(cl_command_queue command_queue, cl_program program, const char* kernel_name, int width, int height, int components, cl_mem input_image, cl_mem result, unsigned long* output) {

  cl_int ret;
  cl_kernel kernel = clCreateKernel(program, kernel_name, &ret);
  CLU_ERRCHECK(ret, "Failed to create kernel from program.");

  unsigned long length = width * height;
  size_t i = 0;

  while (length > 1) {
    size_t global_work_offset = 0;
    size_t local_work_size = 256;
    size_t global_work_size = extend_to_multiple(length, local_work_size);

    cluSetKernelArguments(kernel, 5,
      sizeof(cl_mem), (void*)(i == 0 ? &input_image : &result),
      sizeof(cl_ulong) * local_work_size * components, NULL,
      sizeof(cl_ulong), &length,
      sizeof(cl_int), &components,
      sizeof(cl_mem), (void*)&result
    );

    CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
      &global_work_offset, &global_work_size, &local_work_size, 0, NULL, NULL), "Failed to enqueue 1D kernel.");

    length = global_work_size / local_work_size;
    i++;
  }

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, result,
    CL_TRUE, 0, sizeof(unsigned long) * components, output, 0, NULL, NULL), "Failed reading back result.");

  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue.");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion.");

  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel.");
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: auto_levels [inputfile] [outputfile]\nExample: %s test.png test_out.png\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char const* input_file_name = argv[1];
  const char const* output_file_name = argv[2];

  printf("Loading input file %s …\n", input_file_name);
  int width, height, components;
  unsigned char *data = stbi_load(input_file_name, &width, &height, &components, 0);
  printf("Loaded image of size %d×%d with %d components.\n", width, height, components);

  double start_time = now();

  cl_ulong* image = calloc(width * height * components, sizeof(cl_ulong));

  for(size_t i = 0; i < width * height * components; i++) {
    image[i] = (cl_ulong)data[i];
  }

  cl_int ret;

  cl_command_queue command_queue;
  cl_context context;
  cl_device_id device_id = cluInitDevice(DEVICE_NUMBER, &context, &command_queue);

  size_t image_size = width * height * components * sizeof(*image);
  cl_mem input_image = clCreateBuffer(context, CL_MEM_READ_ONLY, image_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for input image.");

  cl_mem result = clCreateBuffer(context, CL_MEM_READ_WRITE, image_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for result.");

  ret = clEnqueueWriteBuffer(command_queue, input_image, CL_TRUE, 0, image_size, image, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write input image to device.");

  const char* program_file = "kernel.cl";
  cl_program program = cluBuildProgramFromFile(context, device_id, program_file, NULL);

  cl_kernel kernel = clCreateKernel(program, "reduce_sum", &ret);
  CLU_ERRCHECK(ret, "Failed to create kernel from program.");

  unsigned long minimum[components];
  unsigned long maximum[components];
  unsigned long count[components];

  reduce(command_queue, program, "reduce_min", width, height, components, input_image, result, minimum);
  reduce(command_queue, program, "reduce_max", width, height, components, input_image, result, maximum);
  reduce(command_queue, program, "reduce_sum", width, height, components, input_image, result, count);

  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program.");

  // Free device memory.
  CLU_ERRCHECK(clReleaseMemObject(input_image), "Failed to release input buffer.");
  CLU_ERRCHECK(clReleaseMemObject(result), "Failed to release result buffer.");

  // Free management resources.
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue.");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context.");

  for (size_t c = 0; c < components; c++) {
    printf("Component %1u Min: %u\n", c, minimum[c]);
    printf("Component %1u Max: %u\n", c, maximum[c]);
    printf("Component %1u Sum: %u\n", c, count[c]);
  }

  // ------ Analyse Image ------

  // compute min/max/avg of each component
  unsigned char min_val[components];
  unsigned char max_val[components];
  unsigned char avg_val[components];

  // an auxilary array for computing the average
  unsigned long long sum[components];

  // initialize
  for (int c = 0; c < components; c++) {
    min_val[c] = 255;
    max_val[c] = 0;
    sum[c] = 0;
  }

  // compute min/max/sum
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      for (int c = 0; c < components; c++) {
        unsigned char val = data[x * components + y * width * components + c];
        if (val < min_val[c]) min_val[c] = val;
        if (val > max_val[c]) max_val[c] = val;
        sum[c] += val;
      }
    }
  }

  for (int c = 0; c < components; c++) {
    printf("Component %1u Min: %u\n", c, min_val[c]);
    printf("Component %1u Max: %u\n", c, max_val[c]);
    printf("Component %1u Sum: %u\n", c, sum[c]);
  }

  // compute average and multiplicative factors
  float min_fac[components];
  float max_fac[components];
  for (int c = 0; c < components; c++) {
    avg_val[c] = sum[c] / (unsigned long)(width * height);
    min_fac[c] = (float)avg_val[c] / (float)(avg_val[c] - min_val[c]);
    max_fac[c] = (255.0f - (float)avg_val[c]) / (float)(max_val[c] - avg_val[c]);
    printf("\tComponent %1u: %3u / %3u / %3u * %3.2f / %3.2f\n", c, min_val[c], avg_val[c], max_val[c], min_fac[c], max_fac[c]);
  }

  // ------ Adjust Image ------

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      for (int c = 0; c < components; c++) {
        int index = c + x * components + y * width * components;
        unsigned char val = data[index];
        float v = (float)(val - avg_val[c]);
        v *= (val < avg_val[c]) ? min_fac[c] : max_fac[c];
        data[index] = (unsigned char)(v + avg_val[c]);
      }
    }
  }

  printf("Done, took %.1fms\n", (now() - start_time) * 1000.0);

  // ------ Store Image ------

  printf("Writing output image %s …\n", output_file_name);
  stbi_write_png(output_file_name, width, height, components, data, width * components);
  stbi_image_free(data);

  free(image);

  printf("Done!\n");

  // done
  return EXIT_SUCCESS;
}

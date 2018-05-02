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
  cl_mem input_image = clCreateBuffer(context, CL_MEM_READ_WRITE, image_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for input image.");

  cl_mem result = clCreateBuffer(context, CL_MEM_READ_WRITE, image_size, NULL, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for result.");

  ret = clEnqueueWriteBuffer(command_queue, input_image, CL_TRUE, 0, image_size, image, 0, NULL, NULL);
  CLU_ERRCHECK(ret, "Failed to write input image to device.");

  cl_program program = cluBuildProgramFromFile(context, device_id, "kernel.cl", NULL);

  unsigned long minimum[components];
  unsigned long maximum[components];
  unsigned long sum[components];

  reduce(command_queue, program, "reduce_min", width, height, components, input_image, result, minimum);
  reduce(command_queue, program, "reduce_max", width, height, components, input_image, result, maximum);
  reduce(command_queue, program, "reduce_sum", width, height, components, input_image, result, sum);

  // Free device memory.
  CLU_ERRCHECK(clReleaseMemObject(result), "Failed to release result buffer.");

  // ------ Analyse Image ------

  unsigned char average[components];

  // compute average and multiplicative factors
  float minimum_factor[components];
  float maximum_factor[components];
  for (int c = 0; c < components; c++) {
    average[c] = sum[c] / (unsigned long)(width * height);
    minimum_factor[c] = (float)average[c] / (float)(average[c] - minimum[c]);
    maximum_factor[c] = (255.0f - (float)average[c]) / (float)(maximum[c] - average[c]);
    printf("  Component %1u: %3u / %3u / %3u * %3.2f / %3.2f\n", c, minimum[c], average[c], maximum[c], minimum_factor[c], maximum_factor[c]);
  }

  cl_mem minimum_factor_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, components * sizeof(*minimum_factor), minimum_factor, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for input image.");

  cl_mem maximum_factor_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, components * sizeof(*maximum_factor), maximum_factor, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for input image.");

  cl_mem average_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, components * sizeof(*average), average, &ret);
  CLU_ERRCHECK(ret, "Failed to create buffer for input image.");

  cl_kernel kernel = clCreateKernel(program, "adjust", &ret);
  CLU_ERRCHECK(ret, "Failed to create kernel from program.");

  unsigned long length = width * height;

  size_t global_work_offset = 0;
  size_t local_work_size = 256;
  size_t global_work_size = extend_to_multiple(length, local_work_size);

  cluSetKernelArguments(kernel, 6,
    sizeof(cl_mem), (void*)&input_image,
    sizeof(cl_mem), (void*)&minimum_factor_buffer,
    sizeof(cl_mem), (void*)&maximum_factor_buffer,
    sizeof(cl_mem), (void*)&average_buffer,
    sizeof(cl_ulong), &length,
    sizeof(cl_int), &components
  );

  CLU_ERRCHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1,
    &global_work_offset, &global_work_size, &local_work_size, 0, NULL, NULL), "Failed to enqueue 1D kernel.");

  CLU_ERRCHECK(clEnqueueReadBuffer(command_queue, input_image,
    CL_TRUE, 0, sizeof(unsigned long) * width * height * components, image, 0, NULL, NULL), "Failed reading back result.");

  CLU_ERRCHECK(clFlush(command_queue), "Failed to flush command queue.");
  CLU_ERRCHECK(clFinish(command_queue), "Failed to wait for command queue completion.");

  CLU_ERRCHECK(clReleaseKernel(kernel), "Failed to release kernel.");

  CLU_ERRCHECK(clReleaseMemObject(input_image), "Failed to release input buffer.");
  CLU_ERRCHECK(clReleaseMemObject(minimum_factor_buffer), "Failed to release input buffer.");
  CLU_ERRCHECK(clReleaseMemObject(maximum_factor_buffer), "Failed to release input buffer.");
  CLU_ERRCHECK(clReleaseMemObject(average_buffer), "Failed to release input buffer.");

  CLU_ERRCHECK(clReleaseProgram(program), "Failed to release program.");

  // Free management resources.
  CLU_ERRCHECK(clReleaseCommandQueue(command_queue), "Failed to release command queue.");
  CLU_ERRCHECK(clReleaseContext(context), "Failed to release OpenCL context.");

  for(size_t i = 0; i < width * height * components; i++) {
    data[i] = (unsigned char)image[i];
  }

  free(image);

  printf("Done, took %.1fms\n", (now() - start_time) * 1000.0);

  // ------ Store Image ------

  printf("Writing output image %s …\n", output_file_name);
  stbi_write_png(output_file_name, width, height, components, data, width * components);
  stbi_image_free(data);

  printf("Done!\n");

  // done
  return EXIT_SUCCESS;
}

#ifndef __OPEN_CL_H__
#define __OPEN_CL_H__


#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

// -- kernel code utils --

typedef struct kernel_code {
  const char *code;
  size_t size;
} kernel_code;


kernel_code load_code(const char *filename);

void release_code(kernel_code code);

cl_platform_id platform_id;
cl_device_id device_id;
cl_command_queue command_queue;
cl_program program;
cl_kernel kernel;
cl_context context;
cl_uint ret_num_devices;
cl_uint ret_num_platforms;
cl_int ret;

#endif
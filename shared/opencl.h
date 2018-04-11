#ifndef __OPEN_CL_CROSS_PLATFORM_H__
#define __OPEN_CL_CROSS_PLATFORM_H__

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#if __APPLE__
  #include <OpenCL/opencl.h>

  #undef cl_command_queue_properties
  #define cl_command_queue_properties cl_queue_properties_APPLE
  #define clCreateCommandQueueWithProperties clCreateCommandQueueWithPropertiesAPPLE
#else
  #include <CL/cl.h>
#endif

#endif

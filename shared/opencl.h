#ifndef __OPEN_CL_CROSS_PLATFORM_H__
#define __OPEN_CL_CROSS_PLATFORM_H__

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#if __APPLE__
  #include <OpenCL/cl.h>

  #define clCreateCommandQueueWithProperties clCreateCommandQueue

  // #undef CL_MEM_HOST_WRITE_ONLY
  // #define CL_MEM_HOST_WRITE_ONLY 0
  // #undef CL_MEM_HOST_READ_ONLY
  // #define CL_MEM_HOST_READ_ONLY 0
#else
  #include <CL/cl.h>
#endif

#endif

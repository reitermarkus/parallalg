#pragma once

#include <assert.h>
#include <stdarg.h>

#include "opencl.h"

#ifndef _WIN32
  #include <alloca.h>
#endif

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

// check __err for ocl success and print message in case of error
#define CLU_ERRCHECK(__err, ...)                                                 \
  if (__err != CL_SUCCESS) {                                                     \
    fprintf(stderr, "OpenCL Assertion failure in %s#%d:\n", __FILE__, __LINE__); \
    fprintf(stderr, "Error code: %s\n", cluErrorString(__err));                  \
    fprintf(stderr, __VA_ARGS__);                                                \
    fprintf(stderr, "\n");                                                       \
    exit(-1);                                                                    \
  }

#ifndef DEVICE_NUMBER
#define DEVICE_NUMBER 0
#endif

// ------------------------------------------------------------------------------------------------ declarations

// initialize opencl device "num" -- devices are numbered sequentially across all platforms
// if supplied, "command_queue" and "context" are filled with an initialized context and command queue on the device
cl_device_id cluInitDevice(size_t num, cl_context* out_context, cl_command_queue* out_queue);

// like cluInitDevice but with additional support for specifying command queue properties
cl_device_id cluInitDeviceWithProperties(size_t num, cl_context* out_context, cl_command_queue* out_queue, cl_command_queue_properties properties);

// get string with basic information about the ocl device "device" with id "id"
const char* cluGetDeviceDescription(const cl_device_id device, unsigned id);

// loads and builds program from "fn" on the supplied context and device, with the options string "options"
// aborts and reports the build log in case of compiler errors
cl_program cluBuildProgramFromFile(cl_context context, cl_device_id device_id, const char* filename, const char* options);

// sets "num_arg" arguments for kernel "kernel"
// additional arguments need to follow this order: arg0_size, arg0, arg1_size, arg1, ...
void cluSetKernelArguments(const cl_kernel kernel, const cl_uint num_args, ...);

// return string representation of ocl error code "err"
const char* cluErrorString(cl_int err);

// return string representation of ocl device type "type"
const char* cluDeviceTypeString(cl_device_type type);

// ------------------------------------------------------------------------------------------------ implementations

cl_device_id cluInitDevice(size_t num, cl_context* out_context, cl_command_queue* out_queue) {
  return cluInitDeviceWithProperties(num, out_context, out_queue, 0);
}

cl_device_id cluInitDeviceWithProperties(size_t num, cl_context* out_context, cl_command_queue* out_queue, cl_command_queue_properties properties) {
  // get platform ids
  cl_uint ret_num_platforms;
  CLU_ERRCHECK(clGetPlatformIDs(0, NULL, &ret_num_platforms), "Failed to query number of ocl platforms");
  cl_platform_id* ret_platforms = alloca(sizeof(cl_platform_id) * ret_num_platforms);
  CLU_ERRCHECK(clGetPlatformIDs(ret_num_platforms, ret_platforms, NULL), "Failed to retrieve ocl platforms");

  // get device id of desired device
  cl_device_id device_id = NULL;

  #ifdef __CYGWIN__
    ret_num_platforms--;
  #endif

  for (cl_uint i = 0; i < ret_num_platforms; i++) {
    cl_uint ret_num_devices;
    CLU_ERRCHECK(clGetDeviceIDs(ret_platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &ret_num_devices), "Failed to query number of ocl devices");
    if (num < ret_num_devices) {
      // desired device is on this platform, select
      cl_device_id* ret_devices = alloca(sizeof(cl_device_id) * ret_num_devices);
      CLU_ERRCHECK(clGetDeviceIDs(ret_platforms[i], CL_DEVICE_TYPE_GPU, ret_num_devices, ret_devices, NULL), "Failed to retrieve ocl devices");
      device_id = ret_devices[num];
    }

    num -= ret_num_devices;
  }

  // create opencl context if requested
  if (out_context != NULL) {
    cl_int err;
    *out_context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    CLU_ERRCHECK(err, "Failed to create ocl context");

    // create command queue if requested
    if (out_queue != NULL) {
      *out_queue = clCreateCommandQueue(*out_context, device_id, properties, &err);
      CLU_ERRCHECK(err, "Failed to create ocl command queue");
    }
  }

  return device_id;
}

cl_program cluBuildProgramFromFile(cl_context context, cl_device_id device_id, const char* filename, const char* options) {
  FILE* fp;

  // Load the source code containing the kernel.
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel from file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);

  char* source = (char*)calloc(size + 1, sizeof(char));

  if (fread(source, sizeof(char), size, fp) == 0) {
    perror("fread");
    exit(EXIT_FAILURE);
  }

  fclose(fp);

  cl_int err;

  // Create kernel program from source.
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, &err);
  CLU_ERRCHECK(err, "Failed to create program from source file: %s", filename);

  // Build kernel program.
  err = clBuildProgram(program, 1, &device_id, options, NULL, NULL);

  if (err != CL_SUCCESS) {
    fprintf(stderr, "clBuildProgram() failed for source file: %s\n", filename);
    fprintf(stderr, "Error type: %s\n", cluErrorString(err));
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, source, NULL);
    fprintf(stderr, "Build log:\n%s\n", source);
    exit(EXIT_FAILURE);
  }

  free(source);

  return program;
}

void cluSetKernelArguments(const cl_kernel kernel, const cl_uint num_args, ...) {
  // loop through the arguments and call clSetKernelArg for each
  size_t arg_size;
  const void* arg_val;
  va_list arg_list;
  va_start(arg_list, num_args);

  for (cl_uint i = 0; i < num_args; ++i) {
    arg_size = va_arg(arg_list, size_t);
    arg_val = va_arg(arg_list, void*);
    CLU_ERRCHECK(clSetKernelArg(kernel, i, arg_size, arg_val), "Error setting kernel argument %u", i);
  }

  va_end(arg_list);
}

void cluGetDeviceName(const cl_device_id device, const size_t buff_size, char* buffer) {
  CLU_ERRCHECK(clGetDeviceInfo(device, CL_DEVICE_NAME, buff_size, buffer, NULL), "Error getting \"device name\" info");
}

void cluGetDeviceVendor(const cl_device_id device, const size_t buff_size, char* buffer) {
  CLU_ERRCHECK(clGetDeviceInfo(device, CL_DEVICE_VENDOR, buff_size, buffer, NULL), "Error getting \"device vendor\" info");
}

cl_device_type cluGetDeviceType(cl_device_id device) {
  cl_device_type retval;
  CLU_ERRCHECK(clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(retval), &retval, NULL), "Error getting \"device type\" info");
  return retval;
}

#define MAX_DEVICES 16
const char* cluGetDeviceDescription(const cl_device_id device, unsigned id) {
  static char descriptions[MAX_DEVICES][128];
  static cl_bool initialized[MAX_DEVICES];
  assert(id < MAX_DEVICES && "Device limit exceeded");

  if (!initialized[id]) {
    char name[255], vendor[255];
    cluGetDeviceName(device, 255, name);
    cluGetDeviceVendor(device, 255, vendor);
    sprintf(descriptions[id], "%32s  |  Vendor: %32s  |  Type: %4s", name, vendor, cluDeviceTypeString(cluGetDeviceType(device)));
  }

  return descriptions[id];
}

const char* cluDeviceTypeString(cl_device_type type) {
  switch (type) {
    case CL_DEVICE_TYPE_CPU: return "CPU";
    case CL_DEVICE_TYPE_GPU: return "GPU";
    case CL_DEVICE_TYPE_ACCELERATOR: return "ACC";
  }

  return "UNKNOWN";
}

const char* cluErrorString(cl_int err) {
  switch (err) {
    case CL_SUCCESS: return "SUCCESS";
    case CL_DEVICE_NOT_FOUND: return "DEVICE NOT FOUND";
    case CL_DEVICE_NOT_AVAILABLE: return "DEVICE NOT AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE: return "COMPILER NOT AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "MEM OBJECT ALLOCATION FAILURE";
    case CL_OUT_OF_RESOURCES: return "OUT OF RESOURCES";
    case CL_OUT_OF_HOST_MEMORY: return "OUT OF HOST MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return "PROFILING INFO NOT AVAILABLE";
    case CL_MEM_COPY_OVERLAP: return "MEM COPY OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH: return "IMAGE FORMAT MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "IMAGE FORMAT NOT SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE: return "BUILD PROGRAM FAILURE";
    case CL_MAP_FAILURE: return "MAP FAILURE";
    case CL_INVALID_DEVICE_TYPE: return "INVALID DEVICE TYPE";
    case CL_INVALID_PLATFORM: return "INVALID PLATFORM";
    case CL_INVALID_DEVICE: return "INVALID DEVICE";
    case CL_INVALID_CONTEXT: return "INVALID CONTEXT";
    case CL_INVALID_HOST_PTR: return "INVALID HOST PTR";
    case CL_INVALID_MEM_OBJECT: return "INVALID MEM OBJECT";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "INVALID IMAGE FORMAT DESCRIPTOR";
    case CL_INVALID_IMAGE_SIZE: return "INVALID IMAGE SIZE";
    case CL_INVALID_SAMPLER: return "INVALID SAMPLER";
    case CL_INVALID_BINARY: return "INVALID BINARY";
    case CL_INVALID_BUILD_OPTIONS: return "INVALID BUILD OPTIONS";
    case CL_INVALID_PROGRAM: return "INVALID PROGRAM";
    case CL_INVALID_PROGRAM_EXECUTABLE: return "INVALID PROGRAM EXECUTABLE";
    case CL_INVALID_KERNEL_NAME: return "INVALID KERNEL NAME";
    case CL_INVALID_KERNEL_DEFINITION: return "INVALID KERNEL DEFINITION";
    case CL_INVALID_KERNEL: return "INVALID KERNEL";
    case CL_INVALID_ARG_INDEX: return "INVALID ARG INDEX";
    case CL_INVALID_ARG_VALUE: return "INVALID ARG VALUE";
    case CL_INVALID_ARG_SIZE: return "INVALID ARG SIZE";
    case CL_INVALID_KERNEL_ARGS: return "INVALID KERNEL ARGS";
    case CL_INVALID_WORK_DIMENSION: return "INVALID WORK DIMENSION";
    case CL_INVALID_WORK_GROUP_SIZE: return "INVALID WORK GROUP SIZE";
    case CL_INVALID_GLOBAL_OFFSET: return "INVALID GLOBAL OFFSET";
    case CL_INVALID_EVENT_WAIT_LIST: return "INVALID EVENT WAIT LIST";
    case CL_INVALID_EVENT: return "INVALID EVENT";
    case CL_INVALID_OPERATION: return "INVALID OPERATION";
    case CL_INVALID_GL_OBJECT: return "INVALID GL OBJECT";
    case CL_INVALID_BUFFER_SIZE: return "INVALID BUFFER SIZE";
    case CL_INVALID_MIP_LEVEL: return "INVALID MIP LEVEL";
    case CL_INVALID_VALUE: return "INVALID_VALUE";
    case CL_INVALID_QUEUE_PROPERTIES: return "INVALID_QUEUE_PROPERTIES";
    case CL_INVALID_COMMAND_QUEUE: return "INVALID_COMMAND_QUEUE";
  }

  return "UNKNOWN_ERROR";
}

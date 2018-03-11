#ifndef __UTILS_H__
#define __UTILS_H__

#include "headers.h"

// a small wrapper for convenient time measurements

typedef double timestamp;

timestamp now() {
  struct timespec spec;
  timespec_get(&spec, TIME_UTC);
  return spec.tv_sec + spec.tv_nsec / (1e9);
}

#endif
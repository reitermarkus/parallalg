#pragma once

#include <stdbool.h>

#include <time.h>
#include <unistd.h>

// a small wrapper for convenient time measurements

typedef double timestamp;

timestamp now() {
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);
  return spec.tv_sec + spec.tv_nsec / (1e9);
}

size_t extend_to_multiple(size_t value, size_t divisor) {
  size_t rest = value % divisor;

  if (rest == 0) {
    return value;
  }

  return value + (divisor - rest);
}

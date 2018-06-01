#pragma once

size_t extend_to_multiple(size_t value, size_t divisor) {
  size_t rest = value % divisor;

  if (rest == 0) {
    return value;
  }

  return value + (divisor - rest);
}

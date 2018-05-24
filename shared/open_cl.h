#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "opencl.h"

typedef struct kernel_code {
  const char* code;
  size_t size;
} kernel_code;

kernel_code load_code(const char* filename) {
  FILE* fp;

  /* Load the source code containing the kernel*/
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel from file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);

  char* code = (char*)calloc(size + 1, sizeof(char));

  if (fread(code, sizeof(char), size, fp) == 0) {
    perror("fread");
    exit(EXIT_FAILURE);
  }

  fclose(fp);

  return (kernel_code){
    code,
    size,
  };
}

void release_code(kernel_code code) {
  free((char*)code.code);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

int main(int argc, char **argv) {
  srand(0);

  long n = 1000000;
  long count = 0;

  long *array = malloc(sizeof(long) * n);

  for (long i = 0; i < n; i++) {
    array[i] = (rand() % 2);
  }

  timestamp begin = now();

  for (long i = 0; i < n; i++) {
    count += array[i];
  }

  free(array);

  printf("Count: %ld\n", count);

  timestamp end = now();
  printf("Total time: %.3f ms\n", (end - begin) * 1000);

  return EXIT_SUCCESS;
}

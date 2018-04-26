#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  srand(time(NULL));

  long n = 1000000;
  long count = 0;

  long *array = malloc(sizeof(int) * n);

  for (long i = 0; i < n; i++) {
    array[i] = (rand() % 2);
  }

  for (long i = 0; i < n; i++) {
    count += array[i];
  }

  printf("Count: %ld\n", count);

  return EXIT_SUCCESS;
}

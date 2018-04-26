#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  srand(time(NULL));

  long n = 1000000;
  long count = 0;

  long *array = malloc(sizeof(long) * n);

  for (long i = 0; i < n; i++) {
    array[i] = (rand() % 2);
  }

  #pragma omp parallel for reduction(+:count)
  for (long i = 0; i < n; i++) {
    #pragma omp atomic
    count += array[i];
  }

  printf("Count: %ld\n", count);

  return EXIT_SUCCESS;
}

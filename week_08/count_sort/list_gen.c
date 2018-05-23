#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "people.h"
#include "tokenize.h"

void print_list(person_t *list, int size) {
  for (int i = 0; i < size; i++) {
    printf("[%d]\tName: %s\tAge: %d\n", i + 1, list[i].name, list[i].age);
  }
}

void count_sort(person_t *input, int size) {
  int max = 0;

  // step 1) find highest number
  for (int i = 0; i < size; i++) {
    if (input[i].age > max)
      max = input[i].age;
  }
  max++;

  // initialize count array of size max with 0's
  int *count_arr = (int *)calloc(max, sizeof(int));

  // step 2) count occurences
  for (int i = 0; i < size; i++) {
    count_arr[input[i].age]++;
  }

  // step 3) prefix sum
  for (int i = 1; i < max; i++) {
    count_arr[i] = count_arr[i] + count_arr[i-1];
  }

  // initialize a result array
  person_t *result = (person_t *)calloc(size, sizeof(person_t));

  // step 4) insert elements in right order into result array
  for (int i = size - 1; i >= 0; i--) {
    person_t p = input[i];
    result[--count_arr[p.age]] = p;
  }

  memcpy(input, result, sizeof(person_t) * size);
  free(result);
  free(count_arr);
}

void create_person_list(person_t *persons, int n) {
  person_t p;

  for (int i = 0; i < n; i++) {
    p.age = rand() % MAX_AGE;
    strcpy(p.name, gen_name());
    persons[i] = p;
  }
}

int main(int argc, char **argv) {
  int size = 10;
  int seed = 1;

  if (argc > 2) {
    size = atoi(argv[1]);
    seed = atoi(argv[2]);
  }

  srand(seed);
  printf("Generating list of size %d with seed %d\n\n", size, seed);

  person_t *list = (person_t *)malloc(size * sizeof(person_t));
  create_person_list(list, size);

  puts("Unsorted:");
  print_list(list, size);
  
  count_sort(list, size);

  puts("\nSorted:");
  print_list(list, size);

  free(list);
}

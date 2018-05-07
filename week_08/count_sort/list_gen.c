#include <stdio.h>
#include <stdlib.h>

void print(int *array, int size) {
  int i;
  for (i = 0; i < size - 1; i++) {
    printf("%d, ", array[i]);
  }
  printf("%d\n", array[i]);
}

int* count_sort(int *input, int size) {
  int max = 0;

  // find highest number
  for (int i = 0; i < size; i++) {
    if (input[i] > max)
      max = input[i];
  }

  // initialize count array of size max with 0's
  int *count_arr = (int *)calloc(max + 1, sizeof(int));

  // count occurences
  for (int i = 0; i < size; i++) {
    count_arr[input[i]]++;
  }

  for (int i = 0; i <= max; i++) {
    int count = 0;
    for (int j = 0; j < size; j++) {
      if (input[j] < i)
        count++;
    }
    count_arr[i] = count;
  }

  int *result = (int *)calloc(size, sizeof(int));

  for (int i = 0; i < size; i++) {
    int e = input[i];
    result[count_arr[e]++] = e;
  }

  return result;
}

int main(int argc, char **argv) {
  const int size = 8;
  int arr[] = {2, 5, 3, 0, 2, 3, 0, 3};
  int *input = arr;

  puts("Unsorted:");
  print(arr, size);
  
  input = count_sort(arr, size);

  puts("Sorted:");
  print(input, size);

  free(input);
}

typedef char name_t[32];

typedef struct {
	int age;
	name_t name;
} person_t;


kernel void count(global const person_t *input, global int* count_array, const int size) {
  int global_id = get_global_id(0);

  for (int i = 0; i < size; i++) {
    if (input[i].age == global_id) {
      count_array[global_id]++;
    }
  }
  // barrier(CLK_LOCAL_MEM_FENCE);
  // for(int i = 0; i < 10; i++) {
  //   printf("%d\n", count_array[i]);
  // }
}
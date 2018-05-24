typedef char name_t[32];

typedef struct {
	int age;
	name_t name;
} person_t;

kernel void count(global const person_t* input, global ulong* count_array, const ulong max_value, const ulong size) {
  size_t global_id = get_global_id(0);

  if (global_id >= max_value) {
    return;
  }

  count_array[global_id] = 0;

  for (int i = 0; i < size; i++) {
    if (input[i].age == global_id) {
      count_array[global_id]++;
    }
  }
}

kernel void insert(global const person_t* input, global ulong* count_array, global person_t* result, const ulong size) {
  size_t global_id = get_global_id(0);

  for(int i = 0; i < size; i++) {
    if (input[i].age == global_id) {
      result[count_array[global_id]++] = input[i];
    }
  }
}

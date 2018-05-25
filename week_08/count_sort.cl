typedef char name_t[32];

typedef struct {
	int age;
	name_t name;
} person_t;

kernel void count(global const person_t* input, global ulong* count_array, const ulong size) {
  size_t global_id = get_global_id(0);

  if (global_id > size) {
    return;
  }

  atomic_inc(&count_array[input[global_id].age]);
}

kernel void insert(global const person_t* input, global ulong* count_array, global person_t* result, const ulong size) {
  size_t global_id = get_global_id(0);

  if (global_id > size) {
    return;
  }

  result[atomic_inc(&count_array[input[global_id].age])] = input[global_id];
}

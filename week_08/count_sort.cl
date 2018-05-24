typedef char name_t[32];

typedef struct {
	int age;
	name_t name;
} person_t;

kernel void count(global const person_t* input, global int* count_array, const int size) {
  size_t global_id = get_global_id(0);

  for (int i = 0; i < size; i++) {
    if (input[i].age == global_id) {
      count_array[global_id]++;
    }
  }
}

kernel void insert(global const person_t* input, global int* count_array, global person_t* result, const int size) {
  size_t global_id = get_global_id(0);

	for(int i = size - 1; i >= 0; i--) {
		if (input[i].age == global_id) {
      count_array[global_id]--;
      result[count_array[global_id]] = input[i];
    }
	}
}
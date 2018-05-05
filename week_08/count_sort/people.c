#include <stdbool.h>

#include "people.h"

#define loop while (true)

#define lazy_static(type, name, null, init) \
  static type name = NULL;\
  \
  if (name == null) {\
    name = init;\
  }


int count_lines(FILE* file) {
  if (file == NULL) {
    return 0;
  }

  fpos_t current_position;
  assert(fgetpos(file, &current_position) == 0);

  rewind(file);

	int lines = 0;

	char c = '\n';

  loop {
    char previous_char = c;

    c = fgetc(file);

    if (c == '\n') {
      lines++;
    } else if (c == EOF) {
      if (previous_char != '\n') {
        lines++;
      }

      break;
    }
  }

  assert(fsetpos(file, &current_position) == 0);

	return lines;
}

char** load_names(const char *filename, int* lines) {
	FILE *f = fopen(filename, "r");

	*lines = count_lines(f);
	char** storage = (char**)malloc(*lines * sizeof(char*));
	char* space = (char*)malloc(*lines * BUF_SIZE * sizeof(char));

	for(int i = 0; i < *lines; i++) {
		storage[i] = space + i * BUF_SIZE;

		assert(fgets(storage[i], BUF_SIZE-1, f) != NULL);

		// remove whitespace chars, if any
		char *c;
		while((c = strchr(storage[i], '\n'))) *c = '\0';
		while((c = strchr(storage[i], '\r'))) *c = '\0';
		while((c = strchr(storage[i], ' '))) *c = '\0';
	}

	fclose(f);
	return storage;
}

char* gen_name() {
  lazy_static(bool, seeded, false, true; srand(time(0)));

  static int first_name_count, last_name_count;
  lazy_static(char**, first_names, NULL, load_names(FIRST_NAMES_FILE, &first_name_count));
  lazy_static(char**, last_names,  NULL, load_names(LAST_NAMES_FILE,  &last_name_count));

  static name_t buffer;

  snprintf(buffer, NAME_LEN, "%s %s",
    first_names[rand() % first_name_count],
    last_names[rand() % last_name_count]);

  return buffer;
}

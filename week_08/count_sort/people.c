#include <stdbool.h>

#include "people.h"

int count_lines(FILE* file) {
  if (file == NULL) {
    return 0;
  }

  fpos_t current_position;
  fgetpos(file, &current_position);

  rewind(file);

	int lines = 0;

	char c = '\n';

  while (true) {
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

  int ret = fsetpos(file, &current_position);
  assert(ret == 0);

	return lines;
}

int load_names(const char *filename, char ***storage) {
	FILE *f = fopen(filename, "r");

	int lines = count_lines(f);
	*storage = (char**)malloc(lines * sizeof(char*));
	char *space = (char*)malloc(lines * BUF_SIZE * sizeof(char));

	for(int i = 0; i < lines; i++) {
		(*storage)[i] = space + i * BUF_SIZE;

		assert(fgets((*storage)[i], BUF_SIZE-1, f) != NULL);

		// remove whitespace chars, if any
		char *c;
		while((c = strchr((*storage)[i], '\n'))) *c = '\0';
		while((c = strchr((*storage)[i], '\r'))) *c = '\0';
		while((c = strchr((*storage)[i], ' '))) *c = '\0';
	}

	fclose(f);
	return lines;
}

char* gen_name() {
  static bool seeded = false;

  if (!seeded) {
    srand(time(0));
    seeded = true;
  }

  static char** first_names = NULL;
  static char** last_names = NULL;
  static int first_name_count, last_name_count;
  static name_t buffer;

	if (first_names == NULL) { // initialize on first call
		first_name_count = load_names(FIRST_NAMES_FILE, &first_names);
	}

  if (last_names == NULL) {
		last_name_count = load_names(LAST_NAMES_FILE, &last_names);
  }

  snprintf(buffer, NAME_LEN, "%s %s",
    first_names[rand() % first_name_count],
    last_names[rand() % last_name_count]);

  return buffer;
}

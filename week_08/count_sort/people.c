#include <stdbool.h>

#include "people.h"
#include "tokenize.h"

#define lazy_static(type, name, null, init) \
  static type name = NULL;\
  \
  if (name == null) {\
    name = init;\
  }

char** load_names(const char *filename, size_t* lines) {
	FILE *file = fopen(filename, "r");

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* content = malloc((size + 1) * sizeof(char));
  fread(content, sizeof(char), size, file);

  fclose(file);

  return tokenize(content, " \n\r", lines);
}

char* gen_name() {
  lazy_static(bool, seeded, false, true; srand(time(0)));

  static size_t first_name_count, last_name_count;
  lazy_static(char**, first_names, NULL, load_names(FIRST_NAMES_FILE, &first_name_count));
  lazy_static(char**, last_names,  NULL, load_names(LAST_NAMES_FILE,  &last_name_count));

  static name_t buffer;

  snprintf(buffer, NAME_LEN, "%s %s",
    first_names[rand() % first_name_count],
    last_names[rand() % last_name_count]);

  return buffer;
}

#include <stdbool.h>

#include "people.h"
#include "tokenize.h"

static char** first_names = NULL;
static char** last_names = NULL;

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
  static bool seeded = false;

  if (!seeded) {
    srand(time(0));
    seeded = true;
  }

  static size_t first_name_count, last_name_count;

  if (first_names == NULL) {
    first_names = load_names(FIRST_NAMES_FILE, &first_name_count);
  }

  if (last_names == NULL) {
    last_names = load_names(LAST_NAMES_FILE,  &last_name_count);
  }

  static name_t buffer;

  snprintf(buffer, NAME_LEN, "%s %s",
    first_names[rand() % first_name_count],
    last_names[rand() % last_name_count]);

  return buffer;
}

void free_names() {
  free_tokens(first_names);
  free_tokens(last_names);

  first_names = NULL;
  last_names = NULL;
}

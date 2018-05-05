#include "tokenize.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* checked_realloc(void *ptr, size_t size) {
  ptr = realloc(ptr, size);

  if (ptr == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }

  return ptr;
}

char** tokenize(const char* str, const char* sep, size_t* len) {
  if (!str || !sep || !strlen(str) || !strlen(sep)) {
    return NULL;
  }

  char *string = malloc(sizeof(*string) * (strlen(str) + 1));
  strcpy(string, str);

  size_t token_count = 0;
  char** tokens = NULL;

  char* token = NULL;
  char* last_string = NULL;

  for (
    token = strtok_r(string, sep, &last_string);
    token;
    token = strtok_r(NULL, sep, &last_string)
  ) {
    tokens = checked_realloc(tokens, sizeof(*tokens) * ++token_count);
    tokens[token_count - 1] = token;
  }

  tokens = checked_realloc(tokens, sizeof(*tokens) * (token_count + 1));
  tokens[token_count] = NULL;

  if (len != NULL) {
    *len = token_count;
  }

  return tokens;
}

void free_tokens(char** tokens) {
  free(*tokens);
  free(tokens);
}

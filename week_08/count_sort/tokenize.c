#include "tokenize.h"

#include <stdio.h>
#include <stdlib.h>

static void* checked_realloc(void *ptr, size_t size) {
  ptr = realloc(ptr, size);

  if (ptr == NULL) {
    perror("realloc");
    exit(EXIT_FAILURE);
  }

  return ptr;
}

char** tokenize(char* string, const char* sep, size_t* len) {
  if (string == NULL || sep == NULL || !strlen(string) || !strlen(sep)) {
    return NULL;
  }

  size_t token_count = 0;
  char** tokens = NULL;

  char* token = NULL;
  char* last_string = NULL;

  for (
    token = str_split(string, sep, &last_string);
    token;
    token = str_split(NULL, sep, &last_string)
  ) {
    tokens = checked_realloc(tokens, sizeof(tokens) * ++token_count);
    tokens[token_count - 1] = token;
  }

  tokens = checked_realloc(tokens, sizeof(tokens) * (token_count + 1));
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

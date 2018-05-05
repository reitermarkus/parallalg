#pragma once

#include <stddef.h>
#include <string.h>

char** tokenize(char* string, const char* sep, size_t* len);
void free_tokens(char** tokens);

inline char* str_split(char *str, const char *delim, char **saveptr) {
  #ifdef _MSC_VER
    return strtok_s(str, delim, saveptr);
  #else
    return strtok_r(str, delim, saveptr);
  #endif
}

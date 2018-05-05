#pragma once

#include <stddef.h>

char** tokenize(char* string, const char* sep, size_t* len);
void free_tokens(char** tokens);

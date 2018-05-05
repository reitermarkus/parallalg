#pragma once

#include <stddef.h>

char** tokenize(const char* str, const char* sep, size_t* len);
void free_tokens(char** tokens);

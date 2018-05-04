#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_AGE 120
#define NAME_LEN 32

#define BUF_SIZE 16
#define FIRST_NAMES_FILE "first_names.txt"
#define LAST_NAMES_FILE "last_names.txt"

typedef char name_t[NAME_LEN];

typedef struct {
	int age;
	name_t name;
} person_t;

int count_lines(const char *filename);
int load_names(const char *filename, char ***storage);
void gen_name(name_t buffer);

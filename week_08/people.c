#include "people.h"

int count_lines(const char *filename) {
	int lines = 0;
	FILE *f = fopen(filename, "r");
	assert(f != NULL);

	char c;
	while((c = fgetc(f)) != EOF) if(c == '\n') lines++;

	fclose(f);
	if(c != '\n') lines++;

	return lines;
}

int load_names(const char *filename, char ***storage) {
	int lines = count_lines(filename) - 1;
	*storage = (char**)malloc(lines * sizeof(char*));
	char *space = (char*)malloc(lines * BUF_SIZE * sizeof(char));
	
	FILE *f = fopen(filename, "r");

	for(int i = 0; i < lines; i++) {
		(*storage)[i] = space + i * BUF_SIZE;

		assert(fgets((*storage)[i], BUF_SIZE-1, f) != NULL);
		// remove whitespace chars, if any
		char *c;
		while((c = strchr((*storage)[i], '\n'))) *c = '\0';
		while((c = strchr((*storage)[i], '\r'))) *c = '\0';
		while((c = strchr((*storage)[i], ' '))) *c = '\0';
	}

  free(space);
	fclose(f);
	return lines;
}

void gen_name(name_t buffer) {
	static char** first_names = NULL;
	static char** last_names = NULL;
	static int first_name_count, last_name_count;

	if(first_names == NULL) { // initialize on first call
		first_name_count = load_names(FIRST_NAMES_FILE, &first_names);
		last_name_count = load_names(LAST_NAMES_FILE, &last_names);
	}

	snprintf(buffer, NAME_LEN, "%s %s",
		first_names[rand()%first_name_count], 
		last_names[rand()%last_name_count]);
}
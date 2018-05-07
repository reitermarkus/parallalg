CC_FLAGS = -std=c11 -O3 -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=199309L

all: list_gen

list_gen: list_gen.c 
	@$(CC) $(CC_FLAGS) people.c tokenize.c list_gen.c -o list_gen

.PHONEY: clean
clean:
	$(RM) list_gen

run: all
	@./list_gen 



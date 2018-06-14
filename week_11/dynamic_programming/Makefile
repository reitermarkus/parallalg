TARGETS = dynamic_programming_seq dynamic_programming_omp

include ../../shared/shared.mk

CFLAGS +=-O3 -march=native -Werror -pedantic

run: all
	@echo "Sequential:"
	@./dynamic_programming_seq
	@echo ""
	@echo "OpenMP:"
	@./dynamic_programming_omp

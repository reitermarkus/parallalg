TARGETS = mat_mul_bench

include ../../shared/shared.mk

CFLAGS +=-O3 -march=native -Werror -pedantic

run: all
	@echo "Running benchmark .."
	@./mat_mul_bench

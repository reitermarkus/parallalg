TARGETS = dp_seq

include ../../shared/shared.mk

CFLAGS +=-O3 -march=native -Werror -pedantic

run: all
	@./mat_mul_bench

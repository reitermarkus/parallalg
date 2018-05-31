TARGETS = mat_mul_bench

include ../../shared/shared.mk

CC_FLAGS +=-O3 -march=native -Werror -pedantic

run: all
	@echo "Running benchmark .."
	@./mat_mul_bench


OCL_HOME=/scratch/c703/c7031057/opencl

CC=gcc
OPT=-O3
CC_FLAGS= -std=c11 -Werror -pedantic -I$(OCL_HOME)/include -L$(OCL_HOME)/lib

HEADERS=$(wildcard */*.h)
COMMON_DEPENDENCIES=Makefile $(HEADERS)

CFILES=$(wildcard */*.c)

all: mat_mul_seq mat_mul_omp mat_mul_ocl

mat_mul_seq: $(COMMON_DEPENDENCIES)
	@$(CC) $(OPT) $(CC_FLAGS) $(CFILES) mat_mul_seq.c -o mat_mul_seq

mat_mul_omp: $(COMMON_DEPENDENCIES)
	@$(CC) $(OPT) $(CC_FLAGS) $(CFILES) -w mat_mul_omp.c -o mat_mul_omp -fopenmp

mat_mul_ocl: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) mat_mul_ocl.c -o mat_mul_ocl -lOpenCL

.PHONY: clean
clean:
	@rm mat_mul_seq mat_mul_omp mat_mul_ocl
	
run: all
	@echo "Sequential:"
	@./mat_mul_seq
	@echo
	@echo "OpenMP:"
	@./mat_mul_omp
	@echo
	@echo "OpenCL:"
	@./mat_mul_ocl

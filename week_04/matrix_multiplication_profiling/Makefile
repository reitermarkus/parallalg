CC_FLAGS = -std=c11 -O3 -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=199309L
OPENCL_FLAGS = -lOpenCL

ifneq ($(OS), Windows_NT)
  ifeq ($(shell uname -s), Darwin)
    ifeq ($(shell brew ls gcc &>/dev/null; echo $$?), 0)
      GCC_PREFIX = $(shell brew --prefix gcc)
      CC = $(GCC_PREFIX)/bin/gcc-8
      OPENCL_FLAGS = -framework OpenCL
    else
      $(error "GCC is not installed, run `brew install gcc`")
    endif

    # Use dedicated GPU on MacBookPro.
    ifneq (, $(findstring MacBookPro, $(shell /usr/sbin/ioreg -c IOPlatformExpertDevice -r -d 1)))
      OPENCL_FLAGS += -DDEVICE_NUMBER=1
    endif
	else
    OCL_HOME = /scratch/c703/c7031057/opencl
    CC_FLAGS += -I$(OCL_HOME)/include -L$(OCL_HOME)/lib
  endif
endif

HEADERS=$(wildcard */*.h)
COMMON_DEPENDENCIES=Makefile $(HEADERS)

CFILES=$(wildcard */*.c)

all: mat_mul_seq mat_mul_omp mat_mul_ocl

mat_mul_seq: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) mat_mul_seq.c -o mat_mul_seq

mat_mul_omp: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) -w mat_mul_omp.c -o mat_mul_omp -fopenmp

mat_mul_ocl: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) mat_mul_ocl.c -o mat_mul_ocl $(OPENCL_FLAGS)

.PHONY: clean
clean:
	$(RM) mat_mul_seq mat_mul_omp mat_mul_ocl

run: all
	@echo "Sequential:"
	@./mat_mul_seq
	@echo
	@echo "OpenMP:"
	@./mat_mul_omp
	@echo
	@echo "OpenCL:"
	@./mat_mul_ocl

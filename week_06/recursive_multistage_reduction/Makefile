CC_FLAGS = -std=c11 -O3 -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=199309L
OPENCL_FLAGS = -lOpenCL

HEADERS=$(wildcard */*.h)
COMMON_DEPENDENCIES=Makefile $(HEADERS)

CFILES=$(wildcard */*.c)

ifneq ($(OS), Windows_NT)
  ifeq ($(shell uname -s), Darwin)
    ifeq ($(shell brew ls gcc &>/dev/null; echo $$?), 0)
      GCC_PREFIX = $(shell brew --prefix gcc)
      CC = $(GCC_PREFIX)/bin/gcc-7
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

all: parallel_reduction_seq parallel_reduction_omp parallel_reduction_ocl

parallel_reduction_seq: parallel_reduction_seq.c
	@$(CC) $(CC_FLAGS) parallel_reduction_seq.c -o parallel_reduction_seq

parallel_reduction_omp: parallel_reduction_omp.c
	@$(CC) $(CC_FLAGS) parallel_reduction_omp.c -o parallel_reduction_omp -fopenmp

parallel_reduction_ocl: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) parallel_reduction_ocl.c -o parallel_reduction_ocl $(OPENCL_FLAGS)

.PHONEY: clean
clean:
	$(RM) parallel_reduction_seq parallel_reduction_omp parallel_reduction_ocl

run: all
	@echo "Sequential:"
	@./parallel_reduction_seq
	@echo
	@echo "OpenMP:"
	@./parallel_reduction_omp
	@echo
	@echo "OpenCL:"
	@./parallel_reduction_ocl



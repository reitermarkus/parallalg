CC_FLAGS = -std=c11 -O3 -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=199309L
OPENCL_FLAGS = -lOpenCL

HEADERS=$(wildcard */*.h)
COMMON_DEPENDENCIES=Makefile $(HEADERS)

CFILES=$(wildcard */*.c)

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

all: auto_level_seq auto_level_ocl

auto_level_seq: auto_level_seq.c
	@$(CC) $(CC_FLAGS) auto_level_seq.c -lm -o auto_level_seq -fopenmp

auto_level_ocl: $(COMMON_DEPENDENCIES)
	@$(CC) $(CC_FLAGS) $(CFILES) auto_level_ocl.c -lm -o auto_level_ocl $(OPENCL_FLAGS)

.PHONEY: clean
clean:
	$(RM) auto_level_seq auto_level_ocl

earth-huge.png:
	curl -L "http://www.astray.com/static/earth-huge.png" -O

run: all earth-huge.png
	@echo "Sequential:"
	@./auto_level_seq earth-huge.png earth-huge.out.png
	@echo
	@echo "OpenCL:"
	@./auto_level_ocl earth-huge.png earth-huge.out.png



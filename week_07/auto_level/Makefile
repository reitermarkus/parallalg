TARGETS = auto_level_seq auto_level_ocl

CFLAGS += -std=c11 -O3 -Wall -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=200809L

ifneq ($(OS), Windows_NT)
  OS ?= $(shell uname -s)
endif

ifeq ($(OS), Darwin)
  OPENCL_FLAGS = -framework OpenCL

  ifeq ($(shell brew ls gcc@7 &>/dev/null; echo $$?), 0)
    GCC_PREFIX = $(shell brew --prefix gcc@7)
    CC = $(GCC_PREFIX)/bin/gcc-7
  else
    $(error "GCC is not installed, run `brew install gcc@7`")
  endif

  # Use dedicated GPU on MacBookPro.
  ifneq (, $(findstring MacBookPro, $(shell /usr/sbin/ioreg -c IOPlatformExpertDevice -r -d 1)))
    OPENCL_FLAGS += -DDEVICE_NUMBER=1
  endif
else
  OPENCL_FLAGS = -lOpenCL
endif

ifeq ($(OS), Linux)
  OCL_HOME = /scratch/c703/c7031057/opencl
  CFLAGS += -I$(OCL_HOME)/include -L$(OCL_HOME)/lib
endif

LDFLAGS += $(OPENCL_FLAGS) -fopenmp -lm

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONEY: clean
clean:
	$(RM) $(TARGETS) *.o **/*.o

earth-huge.png:
	curl -L "http://www.astray.com/static/earth-huge.png" -O

run: all earth-huge.png
	@echo "Sequential:"
	@./auto_level_seq earth-huge.png earth-huge.out.png
	@echo
	@echo "OpenCL:"
	@./auto_level_ocl earth-huge.png earth-huge.out.png



CFLAGS += -std=c11 -O3 -Wall -Werror -pedantic -I../../shared -D _POSIX_C_SOURCE=200809L

ifneq ($(OS), Windows_NT)
  OS ?= $(shell uname -s)
endif

ifeq ($(OS), Darwin)
  OPENCL_FLAGS = -framework OpenCL

  ifeq ($(shell brew ls llvm &>/dev/null; echo $$?), 0)
    LLVM_PREFIX = $(shell brew --prefix llvm)
    CC = $(LLVM_PREFIX)/bin/clang
    CFLAGS += -I$(LLVM_PREFIX)/include -L$(LLVM_PREFIX)/lib
  else
    $(error "GCC is not installed, run `brew install llvm`")
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

TARGETS = dynamic_programming_seq dynamic_programming_omp dynamic_programming_blocked_omp dynamic_programming_blocked_ocl

include ../../shared/shared.mk

run: all
	@echo "Sequential:"
	@./dynamic_programming_seq
	@echo
	@echo "OpenMP:"
	@./dynamic_programming_omp
	@echo
	@echo "OpenMP (blocked):"
	@./dynamic_programming_blocked_omp
	@echo "OpenCL:"
	@./dynamic_programming_blocked_ocl

TARGETS = mat_mul_seq mat_mul_omp mat_mul_ocl

include ../../shared/shared.mk

run: all
	@echo "Sequential:"
	@./mat_mul_seq
	@echo
	@echo "OpenMP:"
	@./mat_mul_omp
	@echo
	@echo "OpenCL:"
	@./mat_mul_ocl

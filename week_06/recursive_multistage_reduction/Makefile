TARGETS = reduction_seq reduction_omp reduction_ocl

include ../../shared/shared.mk

run: all
	@echo "Sequential:"
	@./reduction_seq
	@echo
	@echo "OpenMP:"
	@./reduction_omp
	@echo
	@echo "OpenCL:"
	@./reduction_ocl



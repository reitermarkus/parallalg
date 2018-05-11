TARGETS = heat_stencil_seq heat_stencil_omp heat_stencil_ocl

include ../../shared/shared.mk

run: all
	@echo "Sequential:"
	@./heat_stencil_seq
	@echo
	@echo "OpenMP:"
	@./heat_stencil_omp
	@echo
	@echo "OpenCL:"
	@./heat_stencil_ocl



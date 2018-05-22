TARGETS = list_gen hillissteele downsweep prefixglobal list_gen_ocl

include ../../shared/shared.mk

$(TARGETS): people.c tokenize.c

run: all
	@./list_gen
	@echo
	@echo 'Hillis & Steele'
	@./hillissteele
	@echo
	@echo 'Downsweep'
	@./downsweep
	@echo
	@echo 'Prefix (global)'
	@./prefixglobal
	@echo 'list_gen_ocl'
	@./list_gen_ocl

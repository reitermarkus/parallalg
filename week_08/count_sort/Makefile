TARGETS = list_gen hillissteele downsweep prefixglobal countsort_bench

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
	@echo 'countsort_bench'
	@./countsort_bench

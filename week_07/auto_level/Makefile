TARGETS = auto_level_seq auto_level_ocl

include ../../shared/shared.mk

earth-huge.png:
	curl -L "http://www.astray.com/static/earth-huge.png" -O

run: all earth-huge.png
	@echo "Sequential:"
	@./auto_level_seq earth-huge.png earth-huge.out.png
	@echo
	@echo "OpenCL:"
	@./auto_level_ocl earth-huge.png earth-huge.out.png



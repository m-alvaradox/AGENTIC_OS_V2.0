# Makefile - AGENTIC OS

.PHONY: all ialearner window launcher clean rebuild

all: ialearner window launcher

ialearner:
	$(MAKE) -C ialearner

window:
	$(MAKE) -C window

launcher:
	$(MAKE) -C launcher

clean:
	$(MAKE) -C ialearner clean
	$(MAKE) -C window clean
	$(MAKE) -C launcher clean

rebuild:
	$(MAKE) -C ialearner rebuild
	$(MAKE) -C window rebuild
	$(MAKE) -C launcher rebuild

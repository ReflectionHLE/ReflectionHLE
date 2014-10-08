.PHONY: clean
all:
	$(MAKE) all -f Makefile.cga
	$(MAKE) all -f Makefile.shareware
clean:
	$(MAKE) clean -f Makefile.cga
	$(MAKE) clean -f Makefile.shareware

.PHONY: clean
all:
	$(MAKE) all -f Makefile.shareware
	$(MAKE) all -f Makefile.cga
	$(MAKE) all -f Makefile.registered
	$(MAKE) all -f Makefile.shareware120
clean:
	$(MAKE) clean -f Makefile.shareware
	$(MAKE) clean -f Makefile.cga
	$(MAKE) clean -f Makefile.registered
	$(MAKE) clean -f Makefile.shareware120

.PHONY: clean
all:
	$(MAKE) all -C src/kdreams
	$(MAKE) all -C src/catabyss
clean:
	$(MAKE) clean -C src/kdreams
	$(MAKE) clean -C src/catabyss

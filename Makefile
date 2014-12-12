.PHONY: clean
all:
	$(MAKE) all -C src/kdreams
	$(MAKE) all -C src/id91_11/cat3d
	$(MAKE) all -C src/id91_11/catadventures/catabyss
clean:
	$(MAKE) clean -C src/kdreams
	$(MAKE) clean -C src/id91_11/cat3d
	$(MAKE) clean -C src/id91_11/catadventures/catabyss

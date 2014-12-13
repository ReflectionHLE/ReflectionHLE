.PHONY: clean
all:
	$(MAKE) all -C src/kdreams
	$(MAKE) all -C src/id91_11/cat3d
	$(MAKE) all -C src/id91_11/catadventures/catabyss
	$(MAKE) all -C src/id91_11/catadventures/catarm
clean:
	$(MAKE) clean -C src/kdreams
	$(MAKE) clean -C src/id91_11/cat3d
	$(MAKE) clean -C src/id91_11/catadventures/catabyss
	$(MAKE) clean -C src/id91_11/catadventures/catarm

all:
	$(MAKE) -C client all
	$(MAKE) -C deamon all

clean:
	$(MAKE) -C client clean
	$(MAKE) -C deamon clean

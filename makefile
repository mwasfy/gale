SUBDIRS=fw mis prk sssp

SW_DIR=$(PWD)/apps

all:
	for dir in $(SUBDIRS); do \
                $(MAKE) -C $(SW_DIR)/$$dir; \
        done

clean:
	for dir in $(SUBDIRS); do \
                $(MAKE) clean -C $(SW_DIR)/$$dir; \
        done


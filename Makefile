SUBDIRS = src test speechsvr

all: $(SUBDIRS)

.PHONY: $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	-for x in $(SUBDIRS); do $(MAKE) -C $$x clean; done

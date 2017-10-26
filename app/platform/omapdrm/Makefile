PROGS=db onoff modesetter testpat planescale capture producer consumer
OMAP_PROGS=omap-db

PKG_CONFIG=pkg-config

ifdef CROSS_COMPILE
	CFLAGS += $(shell $(PKG_CONFIG) --cflags libdrm) $(shell $(PKG_CONFIG) --cflags libdrm_omap)
	LDLIBS += $(shell $(PKG_CONFIG) --libs libdrm) $(shell $(PKG_CONFIG) --libs libdrm_omap)
	PROGS += $(OMAP_PROGS)
else
	CFLAGS += $(shell $(PKG_CONFIG) --cflags libdrm)
	LDLIBS += $(shell $(PKG_CONFIG) --libs libdrm)
endif

CFLAGS += -O2 -Wall -std=c99 -D_GNU_SOURCE -D_DEFAULT_SOURCE -D_XOPEN_SOURCE

LDLIBS += -lrt -pthread
#LDFLAGS += -static

all: $(PROGS)

$(PROGS): % : %.c common.o common-drm.o common-modeset.o common-drawing.o
	@echo "  [LD] $@"
	@$(LINK.c) $^ $(LDLIBS) -o $@

%.o: %.c *.h
	@echo "  [CC] $@"
	@$(COMPILE.c) -o $@ $<

.PHONY: strip clean

strip: $(PROGS)
	$(STRIP) $(PROGS)

clean:
	rm -f $(PROGS) *.o

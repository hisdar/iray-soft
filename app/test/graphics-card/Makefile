CC = arm-linux-gnueabihf-g++

TAG = test-gc

HOME = ../..

INC = -I $(HOME)/include/
INC += -I $(HOME)/include/product/IrayCamera001/
INC += -I /home/caowei/bin/ti-processor-sdk-linux-am57xx-evm-03.03.00.04/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi/usr/include/libdrm/
INC += -I /home/caowei/bin/ti-processor-sdk-linux-am57xx-evm-03.03.00.04/linux-devkit/sysroots/armv7ahf-neon-linux-gnueabi/usr/include/omap/
INC += -I $(HOME)/platform/omapdrm/

LIB = $(HOME)/src/common/*.cpp \
		$(HOME)/src/product/IrayCamera001/*.cpp \
		$(HOME)/src/image/*.cpp \
		$(HOME)/src/services/*.cpp \
		$(HOME)/src/display/*.cpp \
		$(HOME)/platform/omapdrm/common.c \
		$(HOME)/platform/omapdrm/common-drawing.c \
		$(HOME)/platform/omapdrm/common-drm.c \
		$(HOME)/platform/omapdrm/common-modeset.c \
		
SRC = test-gc.cpp

PKG_CONFIG = pkg-config

CFLAGS += $(shell $(PKG_CONFIG) --cflags libdrm) $(shell $(PKG_CONFIG) --cflags libdrm_omap)
CFLAGS += -O3 -Wall -D_GNU_SOURCE -D_DEFAULT_SOURCE -D_XOPEN_SOURCE
CFLAGS += -lpthread -lrt

LDLIBS += $(shell $(PKG_CONFIG) --libs libdrm) $(shell $(PKG_CONFIG) --libs libdrm_omap)

all:
	@$(CC) $(INC) $(LIB) $(SRC) $(LDLIBS) -o $(TAG) $(CFLAGS)

clean:
	rm -rf $(TAG) *.o


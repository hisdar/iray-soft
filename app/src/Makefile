CC = arm-linux-gnueabihf-g++

TAG = iray-camera

HOME = ..

INC = -I $(HOME)/include/ \
		-I $(HOME)/include/product/IrayCamera001/

LIB = $(HOME)/src/common/*.cpp \
		$(HOME)/src/display/*.cpp \
		$(HOME)/src/image/*.cpp \
		$(HOME)/src/net/*.cpp \
		$(HOME)/src/product/IrayCamera001/*.cpp \
		$(HOME)/src/services/*.cpp
		
SRC = iray-camera.cpp

#CFLAGS = -O3 -Wall -std=c99
CFLAGS = -O3 -Wall
CFLAGS += -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib \
		  -I../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/include \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib
CFLAGS += -lpthread -lasound -lrt

all:
	@$(CC) $(INC) $(LIB) $(SRC) -o $(TAG) $(CFLAGS)

clean:
	rm -rf $(TAG) *.o

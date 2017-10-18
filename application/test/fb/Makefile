CC = arm-linux-gnueabihf-g++

TAG = frame-buffer-test

HOME = ../../..

INC = -I $(HOME)/application/lib/include/

LIB = $(HOME)/application/lib/src/FrameBufferDev.cpp \
	$(HOME)/application/lib/src/IrayRgbImage.cpp \
	$(HOME)/application/lib/src/Bt656Merge.cpp \
	$(HOME)/application/lib/src/IrayCameraData.cpp \
		
SRC = main.cpp
		
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

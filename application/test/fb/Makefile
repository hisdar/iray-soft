CC = arm-linux-gnueabihf-g++

TAG = frame-buffer-test

HOME = ../..

SRC = FrameBufferDev.cpp \
		Bt656Merge.cpp \
		IrayRgbImage.cpp \
		main.cpp \
		
CFLAGS = -O3 -Wall
CFLAGS += -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib \
		  -I../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/include \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib
CFLAGS += -lpthread -lasound -lrt

all:
	@$(CC) $(SRC) -o $(TAG) $(CFLAGS)

clean:
	rm -rf $(TAG) *.o

CC = arm-linux-gnueabihf-g++

TAG = net-camera

HOME = ../..

INC = -I $(HOME)/application/lib/include/

LIB = $(HOME)/application/lib/src/IrayCamera.cpp \
		$(HOME)/application/lib/src/IrayCameraData.cpp \
		$(HOME)/application/lib/src/IrayCameraControler.cpp \
		$(HOME)/application/lib/src/NetReceiver.cpp \
		$(HOME)/application/lib/src/IrayImageWriter.cpp \
		$(HOME)/application/lib/src/IrayFrameRate.cpp \

SRC = net-camera.cpp

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

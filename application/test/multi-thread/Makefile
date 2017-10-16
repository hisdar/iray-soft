CC = arm-linux-gnueabihf-g++

TAG = multi-thread-test

HOME = ../..

#INC = -I $(HOME)/application/lib/include/

#LIB = $(HOME)/application/test/fb/FrameBufferDev.cpp \
		
SRC = IrayThread.cpp \
		IrayClientThread.cpp \
		main.cpp

#CFLAGS = -O3 -Wall -std=c99
CFLAGS = -O3 -Wall
CFLAGS += -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib \
		  -I../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/include \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib
CFLAGS += -lpthread -lasound -lrt

all:
	@$(CC) $(SRC) -o $(TAG) $(CFLAGS)
	#@$(CC) $(INC) $(LIB) $(SRC) -o $(TAG) $(CFLAGS)

clean:
	rm -rf $(TAG) *.o

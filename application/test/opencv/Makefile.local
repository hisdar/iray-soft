TARGET = opencv-resize

HOME = ../../..

INCLUDE = -I $(HOME)/application/lib/include/

LIBARY = $(HOME)/application/lib/src/*.cpp

SOURCE = main.cpp

CXX = arm-linux-gnueabihf-g++
CFLAGS += -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib \
		  -I../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/include \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib
CFLAGS +=  -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lOpenCL -lpthread -lrt

all:
	@$(CXX) $(INCLUDE) $(LIBARY) $(SOURCE) -o $(TARGET) $(CFLAGS)

clean:
	rm -rf $(TARGET) *.o

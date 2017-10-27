TARGET = vide0-encode
CXX = arm-linux-gnueabihf-g++
CFLAGS += -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -Wl,-rpath-link,../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib \
		  -I../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/include \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/usr/lib \
		  -L../../../linux-devkit/sysroots/cortexa15hf-neon-linux-gnueabi/lib
CFLAGS +=  -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lOpenCL -lpthread -lrt

all:
	@$(CXX)  main.cpp -o $(TARGET) $(CFLAGS)

clean:
	rm -rf $(TARGET) *.o


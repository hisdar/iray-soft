CC = arm-linux-gnueabihf-g++

TAG = test-vpe

HOME = ../..

INC = -I $(HOME)/include/
INC += -I $(HOME)/include/product/IrayCamera001/

LIB = $(HOME)/src/common/*.cpp \
		$(HOME)/src/product/IrayCamera001/*.cpp \
		$(HOME)/src/image/*.cpp \
		$(HOME)/src/services/*.cpp \

SRC = TestVpe.cpp

CFLAGS = -O3 -Wall
CFLAGS += -lpthread -lrt

all:
	@$(CC) $(INC) $(LIB) $(SRC) -o $(TAG) $(CFLAGS)

clean:
	rm -rf $(TAG) *.o


UNAME_S :=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
CCFLAGS += -fpic
endif

TARGET_DIR = inst

all: install
install: $(TARGET_DIR) inst/clientCmd.o 
	g++ -g -std=c++11  ../Common/inst/protocol.o ../Common/inst/clientToServer.o inst/clientCmd.o -o inst/runClient -lpthread


inst/clientCmd.o: clientCmd.cpp
	g++ -g -std=c++11 -c clientCmd.cpp -o inst/clientCmd.o 
	
$(TARGET_DIR): 
			test ! -d $(TARGET_DIR) && mkdir $(TARGET_DIR)

clean:
	rm inst/*


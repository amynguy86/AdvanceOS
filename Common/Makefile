UNAME_S :=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
CCFLAGS += -fpic
endif

TARGET_DIR = inst

all: install
install: $(TARGET_DIR) inst/protocol.o inst/clientToServer.o

inst/protocol.o: protocol.cpp
	g++ -g -std=c++11 $(CCFLAGS) -c protocol.cpp -o inst/protocol.o 

inst/clientToServer.o: clientToServer.cpp
	g++ -g -std=c++11 $(CCFLAGS) -c clientToServer.cpp -o inst/clientToServer.o 

$(TARGET_DIR):
			test ! -d $(TARGET_DIR) && mkdir $(TARGET_DIR)
clean:
	rm inst/*

UNAME_S :=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
CCFLAGS += -fpic
endif

TARGET_DIR = inst

all: install
install: $(TARGET_DIR) inst/serverToServer.o inst/memory.o inst/incoming.o inst/outgoing.o inst/start.o
	g++ -g -std=c++11  ../Common/inst/clientToServer.o ../Common/inst/protocol.o  inst/start.o inst/serverToServer.o inst/memory.o inst/incoming.o inst/outgoing.o -o inst/runServer -lpthread

inst/serverToServer.o: serverToServer.cpp
	g++ -g -std=c++11 -c serverToServer.cpp -o inst/serverToServer.o -lpthread 

inst/memory.o:	memory.cpp
	g++ -g -std=c++11 -c memory.cpp -o inst/memory.o 

inst/incoming.o: incoming.cpp
	g++ -g -std=c++11 -c incoming.cpp -o inst/incoming.o  -lpthread

inst/outgoing.o: outgoing.cpp
	g++ -g -std=c++11 -c outgoing.cpp -o inst/outgoing.o  -lpthread

inst/start.o: start.cpp
	g++ -g -std=c++11 -c start.cpp -o inst/start.o  -lpthread

$(TARGET_DIR):
			test ! -d $(TARGET_DIR) && mkdir $(TARGET_DIR)
clean:
	rm inst/*

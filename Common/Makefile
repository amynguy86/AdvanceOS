UNAME_S :=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
CCFLAGS += -fpic
endif

all: install
install: inst/protocol.o inst/clientToServer.o

inst/protocol.o: protocol.cpp
	g++ -g -std=c++11 $(CCFLAGS) -c protocol.cpp -o inst/protocol.o 

inst/clientToServer.o: clientToServer.cpp
	g++ -g -std=c++11 $(CCFLAGS) -c clientToServer.cpp -o inst/clientToServer.o 

clean:
	rm inst/*

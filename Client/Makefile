UNAME_S :=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
CCFLAGS += -fpic
endif

all: install
install: inst/clientCmd.o
	g++ -g -std=c++11  ../Common/inst/protocol.o ../Common/inst/clientToServer.o inst/clientCmd.o -o inst/runClient -lpthread


inst/clientCmd.o: clientCmd.cpp
	g++ -g -std=c++11 -c clientCmd.cpp -o inst/clientCmd.o 
clean:
	rm inst/*


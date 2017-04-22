#ifndef INCOMING_H
#define INCOMING_H

#define BACKLOG 10 

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "outgoing.h"
#include "memory.h"
using namespace std;
typedef unordered_map<Id,struct sockaddr_in> ProcessIPMap;

class Incoming;

class Obj_method {
 public:
 Incoming * obj;
 int fd; 
};

class Thread_fd { 
 public:
 std::thread * myThread;
 int fd; //fd the thread is listening on
};

class Incoming
{
public:
int bindToIp(const char * const myIP,const char * port=NULL,bool isForClient=false);
int start();
void stop();
bool isStopped(){return stopped;}
int waitForConn();
struct sockaddr_in& getMyIP(){return myIP;}
void joinAllThreads();
int allowClientRqsts();
static void serveClientRqsts(Obj_method obj);
/* 
 * Attributes that must be set
 */
void setTotalProc(int totalProc){this->totalProc=totalProc;}
int getTotalProc(){return totalProc;}
void setMyProcNum(Id proc){myProcNum=proc;}
Id getMyProcNum(){return myProcNum;}
void setOutgoingHandler(Outgoing * outgoing){outgoingHandler=outgoing;}
Outgoing * getOutgoingHandler(){return outgoingHandler;}
static void communicate(Obj_method obj_method);
ProcessIPMap * getProcessIPMap(){return &processIPMap;}
Incoming()
{
 stopped=true;
 serverFd=-1;
 totalProc=0;
 numCompleteRcvd=0;
 clientFd=-1;
 serverFd=-1;
}
~Incoming();

int numCompleteRcvd;

private:
int serverFd;
int clientFd; //fd on which listeing for clients to connect

static std::mutex ivMut; //mutex
static std::condition_variable ivCond;
Id myProcNum;
ProcessIPMap processIPMap;
vector<Thread_fd> myThread_fds;
bool stopped;
struct sockaddr_in myIP;
int totalProc;
Outgoing * outgoingHandler;
bool shutDown;
};
#endif

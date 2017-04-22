#ifndef OUTGOING_H
#define OUTGOING_H

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include "serverToServer.h"
#include "../Common/clientToServer.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

/*
 * Defintion of Global Variables defined in the Main Unit
 */
typedef unordered_map<Id,int> ProcessFDMap ; //processNumeber map to socket
typedef unordered_map<Id,int> FdProcessMap ; //socket to ProcessNumber

typedef ServerToServer MsgParser;

class Outgoing
{
public:
int establishConnWithMain(); 
int establishConnWithAll(); 
int tearDown();
void setProcessNum(Id aProcessNum){processNum=aProcessNum;if(processNum==0)ivMut.lock();}
void setMyAddr(struct sockaddr_in & aMyAddr){myAddr=aMyAddr;}
void setMainProcAddr(struct sockaddr_in & aAddr){mainProcAddr=aAddr;}
void setTotalProc(int num){totalProc=num;}
void addToProcessFDMap(Id processNum,int fd);

Id convertFdtoId(int fd){return fdProcessMap[fd];}
int send(Id toProcessNum,ServerToServer &reqMsgtoServer);

static bool allTrue(bool * toCheck,int size);
void stop();
Outgoing(){processNum=0;toreDown=false;}
~Outgoing(){;}
bool toreDown;

private:
int clientFd;
Id processNum;
MsgParser msgParser;
ServerToServer sendMsg;
struct sockaddr_in mainProcAddr;
struct sockaddr_in myAddr;
ProcessFDMap processFDMap;
FdProcessMap fdProcessMap;
std::mutex ivMut;
int totalProc;

};
#endif

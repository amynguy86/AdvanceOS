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

typedef struct
{
 int fd;
 bool enabled;
} FDHolder;

typedef unordered_map<Id,FDHolder> ProcessFDMap ; //processNumeber map to socket
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

//return 0 for success, -1 for failure, anyone sending should already ensure
//that link is avaliable

int send(Id toProcessNum,ServerToServer &reqMsgtoServer);
int sendMsgToServers(const vector<Id> &serverToSend,ServerToServer reqMsgToServer);

void disableLink(Id processNum){processFDMap[processNum].enabled=false;}
bool isLinkDisabled(Id processNum){return !processFDMap[processNum].enabled;}

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
std::mutex ivMut; //this is first used on startup and then to protect disable link flag.
int totalProc;

};
#endif

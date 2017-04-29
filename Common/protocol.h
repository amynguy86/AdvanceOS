/* This is the base class for protocol utlized for Client to Server Communication
 * and Server to Server Communication
 *
 * Protocal Header
 *
 * Length: two byte
 * (Server/Client) ID: one byte
 * ProtocolType: one byte 
 * Code: one byte 
 * RqstNo: onebyte(helps to match the rqst with the reply)
 * ResultCode: one byte(set to None when not applicable)
 *
 * MessageBody
 *  Inclusion and interpretation of this block of data depends on the code/ProtocolType)
 *  DataBlock:
 *  Key: (4bytes)
 *  ValueLen: (2bytes)
 *  Value: (ValueLen)
 *
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include <netinet/in.h>
#include <cstring>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

typedef std::unordered_map<unsigned char,struct sockaddr_in> IpAddrColl;

//Header Types
typedef unsigned short  Length;
typedef unsigned char Id; 
typedef unsigned char ProtocolType;
typedef unsigned char Code; 
typedef unsigned char ResultCode;
typedef unsigned char RqstNo;

//Sizez of stuff in header
static const int SIZEOFLEN=sizeof(Length);
static const int SIZEOFID=sizeof(Id);
static const int SIZEOFPROTOCOLTYPE=sizeof(ProtocolType);
static const int SIZEOFCODE=sizeof(Code);
static const int SIZEOFRC=sizeof(ResultCode);
static const int SIZEOFRQSTNO=sizeof(RqstNo);

//Stuff in DataBlock
typedef unsigned int Key;
typedef unsigned short ValLen;


//Total sizes=
static const int HEADERLEN=SIZEOFLEN+SIZEOFID+SIZEOFPROTOCOLTYPE+SIZEOFCODE+SIZEOFRC+SIZEOFRQSTNO; //7 bytes
static const int MAX_PACKETLEN=65535; 
static const int MIN_PACKETLEN=HEADERLEN; 
static const int MAX_DATASIZE=65535;

//Result Codes
static const int RC_SUCCESS=1;
static const int RC_FAIL=2;
static const int RC_NONE=0;

//Protocol Type
static const int SERVERTOSERVER=0;
static const int CLIENTTOSERVER=1;

//Codes
static const int CODE_ESTABLISH=0;
static const int CODE_ESTABLISHACK=1;
static const int CODE_INSERT=2;
static const int CODE_UPDATE=3;
static const int CODE_NONE=4;
static const int CODE_REPLY=5;
static const int CODE_READ=6;
static const int CODE_COMPLETE=7;
static const int CODE_DELETE=8;
static const int CODE_GETMYID=9;
static const int CODE_VOTE_REQUEST=10;
static const int CODE_VOTE_REPLY=11;

//Structure represeting the DataBlock
typedef struct 
{
 Key key;
 ValLen valLen;
 const char * val;
} DataBlock;

class Protocol
{
protected:
virtual int addDataBlock(unsigned short& writtenSoFar);

Code code;
Id id;
ProtocolType protocolType;
ResultCode rc;
Length msgLen;
RqstNo rqstNo;
DataBlock dataBlock;
char buffer[MAX_PACKETLEN];
bool msgCreated;

public:
/* These method do the background work to make sure the entire packet
 * is sent or received
 *

 *  0: Successful
 * -1: Error
 *  1: Connection closed by host
*/
int sendAll(int fd,bool createAgain=true);
int recvAll(int fd);

void setCode(Code aCode){code=aCode;}
void setId( Id anId){id=anId;}
void setRc(ResultCode aRc){rc=aRc;}
void setRqstNo(RqstNo num){rqstNo=num;}

unsigned short getMsgLen(){return msgLen;}
Code getCode(){return code;};
Id getId(){return id;}
ResultCode getRc(){return rc;}
Key getKey(){return dataBlock.key;}
const char * getVal(){return dataBlock.val;}
ValLen getValLen(){return dataBlock.valLen;}
RqstNo getRqstNo(){return rqstNo;}
virtual void reset();
virtual int parseDataBlock(int & readSoFar);

void setDataBlock(Key key, ValLen valLen=0,const char * val=NULL);

Protocol();
~Protocol(){}

private:
void setMsgLen(unsigned short aMsgLen){msgLen=aMsgLen;}
char const * const createMsg();

//Must return -1 failure, bodysize for success
virtual int createBody()=0;

/*
 * Returns the remaining expected bytes or -1 for failure
 */
int parseHeader();

//Must return -1 failure, 0 for success
virtual int parseMsg()=0;
char * getBuffer(){return buffer;}
};
#endif

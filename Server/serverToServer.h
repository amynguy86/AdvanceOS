/* This is the base class for protocol utlized for Server to Server Communication
 * which includes setting up all the servers via establish
 *
 * MessageBody
 * 
 * When code is Establish
 * onebyte(procNumber),4bytes(IpAddress),2bytes(Port)
 *
 * When code is EstablishAck
 * onebyte(procNumber),4bytes(IpAddress),2bytes(Port)
 * onebyte(procNumber),4bytes(IpAddress),2bytes(Port)
 * ...
 * ...
 * 
 */

#ifndef SERVERTOSERVERL_H
#define SERVERTOSERVER_H
#include "../Common/protocol.h"
#include "memory.h"

static const int SIZEOF_IPADDR_BYTE=sizeof(Id)+sizeof(in_addr)+sizeof(short);//1,192.168.1.1,1812

class ServerToServer : public Protocol
{
public:
void addIpAddr(struct  sockaddr_in * addr, Id aProcessNum){allAddrs[aProcessNum]=*addr;}
const IpAddrColl *  getAllIpAddr(){return &allAddrs;}
void setAllIpAddr(const IpAddrColl& ipAddrColl){allAddrs=ipAddrColl;}
void reset();
void setMyAddr(struct sockaddr_in& anAddr){addr=anAddr;}
const struct sockaddr_in&  getAddr(){return addr;}
void printAllIPAddr(FILE * stream);
void setMetaData(RU ru,Version version,DS ds){metaData.ds=ds;metaData.version=version;metaData.ru=ru;}
const Memory::Holder::MetaData& getMetaData(){return metaData;}

ServerToServer();
~ServerToServer(){}

private:
int parseMsg();
int extractAllIpAddr(const char * const buffer,int fromLen, int totalLen);
void getIPAddr(struct sockaddr_in&, char const * const buffer,int fromLen,Id& p); //this method does not verify length of the buffer is indeed SIZEIF_IPADDR_BYTE
int createBody();
int parseMetaBlock(int& readSoFar);
int addMetaBlock(unsigned short& writtenSoFar);

IpAddrColl allAddrs; //when code is CODE_ESTABLISHACK
struct sockaddr_in addr;
Memory::Holder::MetaData metaData;
};
#endif

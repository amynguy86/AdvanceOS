/* 
 * This is the representation of memory. Memory here is just cache memory
 * data resides in a memory in object called holder
 * The hash is protected with a mutex.
 * holder data is also protected with a with a different mutex
 *
 * This class is also a singleton Class so that memory can be accessed
 * anywhere. 
 *
 * The holder object also allows to detect if the required number of messages are
 * receieved from the replicated servers before the data is unlocked
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <sstream>
#include <string>
#include <assert.h>
#include <mutex>



class Memory
{
public:
//-1 for error
//1 for success
/*
 * The one with the lock will only be executed by the primary server of the object.
 * The other one will be executed by the backup server of the object upon receving the request from the primary server.
 */ 
int AddOrUpdateWithLock(unsigned int key,const char * val,int initiatingClient,unsigned char serverToWaitOnBitMap,int rqstNo,bool isUpdate);
int AddOrUpdate(unsigned int key,const char * val,bool isUpdate);



const char * read(unsigned int key);
static Memory * getInstance();
~Memory(){}

class Holder
{
public:
Holder(){;}
~Holder(){;}
std::string currData;
std::string newData;

void commit(){currData=newData;}

class MetaData
{
public:
unsigned char serverWaitingOnBitMap;
int initiatingClient;
int rqstNo;
bool isUpdate; 
};

MetaData metaData;
std::mutex ivMut;
};

//-1 for error
//0 for success
//1 for still waiting on clients
int unlockData(unsigned int key,int rqstNo,unsigned char FromServerNo,Holder::MetaData &metaData);
private:
Memory(){;}

std::unordered_map<unsigned int,Holder *> cache;
std::mutex ivMut;
Memory::Holder holder;
};
#endif

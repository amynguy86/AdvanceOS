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

typedef unsigned char RU;
typedef unsigned char DS;
typedef unsigned char Version;


class Memory
{
public:

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
unsigned char serverWaitingOnUnchanged;
int initiatingClient;
int rqstNo;
bool isUpdate; 

/*
 * Project 3 specific variables
 */
 std::unordered_map<unsigned char,MetaData > incoming_metadata;// new hash table to hold the data from other servers..
 
 DS ds;
 RU ru;
 Version version;
};

MetaData metaData;
std::mutex ivMut;
};

//-1 for error
//1 for success
/*
 * The one with the lock will only be executed by the primary server of the object.
 * The other one will be executed by the backup server of the object upon receving the request from the primary server.
 */
int AddOrUpdateWithLock(unsigned int key,const char * val,int initiatingClient,unsigned char serverToWaitOnBitMap,int rqstNo,bool isUpdate);
int AddOrUpdate(unsigned int key,const char * val,const Holder::MetaData& meta,bool isUpdate);



const char * read(unsigned int key);
int readMetaData(unsigned int key,RU &ru, Version &version,DS &ds);
static Memory * getInstance();
~Memory(){}



//-1 for error
//0 for success
//1 for still waiting on more server replies
//2 Unable to form majority

//MetaDataOut is the metadata copied from whats in the memory
//MetaDataOut is the metaData passed from the server that replied with CODE_VOTE_REPLY

//Use metaDataOut for keeping track of which servers contains the latest version No.
int unlockData(unsigned int key,std::string& val,int rqstNo,unsigned char FromServerNo,const Holder::MetaData &metaDataIn,Holder::MetaData &metaDataOut);
int forceUnlockData(int key);
private:
Memory();

std::unordered_map<unsigned int,Holder *> cache;
std::mutex ivMut;
Memory::Holder holder;
};
#endif

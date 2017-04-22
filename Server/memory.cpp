#include "memory.h"

int Memory::AddOrUpdateWithLock(unsigned int key,const char * val,int initiatingClient,unsigned char serverToWaitOnBitMap,int rqstNo,bool isUpdate)
{
 Memory::Holder * data= NULL;
 {
  std::lock_guard<std::mutex> lck (ivMut);
  
  try
  {
   data=(Memory::Holder *)cache.at(key);
   if(!isUpdate)
    {
     fprintf(stderr,"Its an Insert, and there is already a value mapped to key: %d\n",key);
     return -1;
    }
  }
  catch(const std::out_of_range& oor)
  {
  if(isUpdate)
   {
   fprintf(stderr,"Its an Update, and could not find value mapped by key %d\n",key);
   return -1;
   }
  else   {
    data=new Memory::Holder();
    cache[key]=data;
   }
  }
 }
  data->ivMut.lock(); 
  data->metaData.initiatingClient=initiatingClient;
  data->metaData.serverWaitingOnBitMap=serverToWaitOnBitMap;
  data->metaData.rqstNo=rqstNo;
  data->metaData.isUpdate=isUpdate;
  data->newData=val;
 
 return 0;
}

int Memory::AddOrUpdate(unsigned int key,const char * val,bool isUpdate)
{
  Memory::Holder * data= NULL;
  std::lock_guard<std::mutex> lck (ivMut);
  
  try
  {
   data=(Holder *)cache.at(key);
   if(!isUpdate)
    {
     fprintf(stderr,"Its an Insert(AddOrUpdate()), and there is already a value mapped to key: %d\n",key);
     return -1;
    }
  }
  catch(const std::out_of_range& oor)
  {
  if(isUpdate)
   {
   fprintf(stderr,"Its an Updat(AddOrUpdate(), and could not find value mapped by key %d\n",key);
   return -1;
   }
  else   
   {
    data=new Holder();
    cache[key]=data;

   }
  }
 data->currData=val;
}

int Memory::unlockData(unsigned int key,int rqstNo,unsigned char fromServerNo,Holder::MetaData &metaData)
{
 Holder * data= NULL;
 {
 std::lock_guard<std::mutex> lck (ivMut);
 try
  {
   data=cache.at(key);
  } 
   catch(const std::out_of_range& oor)
  {
   fprintf(stderr,"unLockData, and could not find value mapped by key %d\n",key);
   return -1;
  }
 
  if(data->ivMut.try_lock())
  { 
   fprintf(stderr,"unLockData, Objected mapped by key: %d not locked\n",key);
   data->ivMut.unlock();
   return -1;
  }
 
 if(rqstNo!=data->metaData.rqstNo)
  {
   fprintf(stderr,"unLockData, rqstNo does not match RqstNo given%d: acual RqstNo%d: key: %d\n",rqstNo,metaData.rqstNo,key);
   return -1;
  }
 
 data->metaData.serverWaitingOnBitMap= (data->metaData.serverWaitingOnBitMap & ~fromServerNo);
 
 if(data->metaData.serverWaitingOnBitMap!=0)
  {
  fprintf(stderr,"Still Waiting for more replies from server\n");
  return 1;
  }
 }

 metaData.initiatingClient=data->metaData.initiatingClient;
 metaData.rqstNo=data->metaData.rqstNo;
 metaData.isUpdate=data->metaData.isUpdate;
 data->commit();
 data->ivMut.unlock();
 return 0;
}

const char * Memory::read(unsigned int key)

{
 Holder * data=NULL;
 std::lock_guard<std::mutex> lck (ivMut);
 try
  {
   data=cache.at(key);
  }
  catch(const std::out_of_range& oor)
  {
   fprintf(stderr,"Its an Read, and could not find value mapped by key %d\n",key);
   return NULL;
  }
  
  return data->currData.c_str();
}

Memory * Memory::getInstance()
{
 static Memory * memory=NULL;
 if(memory==NULL)
  memory= new Memory();
 
 return memory;
}

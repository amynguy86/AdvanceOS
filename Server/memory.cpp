#include "memory.h"

Memory::Memory()
{
	//The first time Memory is initialized, its add object X[Key 0] into itself
	Holder * data;
	data=new Holder();
	data->currData="X";
	data->metaData.ds=0;
	data->metaData.version=1;
	data->metaData.ru=8;
	cache[0]=data;
}
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
  data->metaData.serverWaitingOnUnchanged=serverToWaitOnBitMap;
  data->metaData.rqstNo=rqstNo;
  data->metaData.isUpdate=isUpdate;
  data->newData=val;
 
 return 0;
}

int Memory::AddOrUpdate(unsigned int key,const char * val,const Holder::MetaData& meta,bool isUpdate)
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
   fprintf(stderr,"Its an Update(AddOrUpdate(), and could not find value mapped by key %d\n",key);
   return -1;
   }
  else   
   {
    data=new Holder();
    cache[key]=data;

   }
  }
 data->currData=val;
 data->metaData.ds=meta.ds;
 data->metaData.ru=meta.ru;
 data->metaData.version=meta.version;

 //the following output is graded!!
 std::cout<<"Key:"<<key<<" Val:"<<val<<std::endl;
 std::cout<<"RU:"<<(int)data->metaData.ru<<std::endl;
 std::cout<<"Version:"<<(int)data->metaData.version<<std::endl;
 std::cout<<"DS:"<<(int)data->metaData.ds<<std::endl;

 return 0;
}

int Memory::forceUnlockData(int key)
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
			fprintf(stderr,"forceUnLockData, and could not find value mapped by key %d\n",key);
			return -1;
		}
	}
	data->ivMut.unlock();
	return 0;
}

int Memory::unlockData(unsigned int key,std::string& val,int rqstNo,unsigned char fromServerNo,const Holder::MetaData &metaDataIn,Holder::MetaData &metaDataOut)
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
   fprintf(stderr,"unLockData, rqstNo does not match RqstNo given%d: acual RqstNo%d: key: %d\n",rqstNo,data->metaData.rqstNo,key);
   return -1;
  }
 
 data->metaData.serverWaitingOnBitMap= (data->metaData.serverWaitingOnBitMap & ~fromServerNo);
 
 if(data->metaData.serverWaitingOnBitMap!=0)
  {
  fprintf(stderr,"Still Waiting for more replies from server\n");
  return 1;
  }
 }

 metaDataOut.initiatingClient=data->metaData.initiatingClient;
 metaDataOut.rqstNo=data->metaData.rqstNo;
 metaDataOut.isUpdate=data->metaData.isUpdate;
 metaDataOut.serverWaitingOnUnchanged=data->metaData.serverWaitingOnUnchanged;

 /*
  * Need to evaluate here if majorty is formed(todo)
  * if majority not formed, should not commit however please unlock it anyways
  */

 /*
  * Unit Test
  */
 bool majorityFormed=true;

 if(!majorityFormed)
 {
	 fprintf(stderr,"Unable To Form majority\n");
	 data->ivMut.unlock();
	 return 2;
 }
 else
 {
	data->metaData.version++;
	metaDataOut.ru=data->metaData.ru;
	metaDataOut.ds=data->metaData.ds;
	metaDataOut.version=data->metaData.version;
	data->commit();
	val=data->currData;
 }

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

int Memory::readMetaData(unsigned int key,RU &ru, Version &version,DS &ds)
{
 Holder * data=NULL;
 std::lock_guard<std::mutex> lck (ivMut);
 try
  {
   data=cache.at(key);
  }
  catch(const std::out_of_range& oor)
  {
   fprintf(stderr,"Its an readMeta, and could not find value mapped by key %d\n",key);
   return -1;
  }

  version=data->metaData.version;
  ru=data->metaData.ru;
  ds=data->metaData.ds;
  return 0;
}

Memory * Memory::getInstance()
{
 //todo Need Mutex here but ok for now
 static Memory * memory=NULL;
 if(memory==NULL)
  memory= new Memory();
 
 return memory;
}

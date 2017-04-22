#include "protocol.h"

Protocol::Protocol()
{
 reset();
}

void Protocol::reset()
{
 msgLen=0;
 msgCreated=false;
 memset(buffer,0,MAX_PACKETLEN);
 id=0;
 rc=RC_NONE;
 dataBlock.valLen=0;
 dataBlock.key=0;
 dataBlock.val=NULL;
}

int Protocol::parseHeader()
{

  int readSoFar=0;

  std::memcpy(&msgLen,buffer+readSoFar,sizeof(msgLen));
  msgLen = ntohs(msgLen);
  
  readSoFar+=sizeof(msgLen);
  
  std::memcpy(&id,buffer+readSoFar,sizeof(id));
  readSoFar+=sizeof(id);
  
  std::memcpy(&protocolType,buffer+readSoFar,sizeof(protocolType));
  readSoFar+=sizeof(protocolType);
  
  std::memcpy(&code,buffer+readSoFar,sizeof(code));
  readSoFar+=sizeof(code);
  
  std::memcpy(&rqstNo,buffer+readSoFar,sizeof(rqstNo));
  readSoFar+=sizeof(rqstNo);
  
  std::memcpy(&rc,buffer+readSoFar,sizeof(rc));
  readSoFar+=sizeof(rc);

  if(msgLen<MIN_PACKETLEN)
   return -1;

  return msgLen-readSoFar;
}

int Protocol::sendAll(int fd,bool creatMsg)
{
 if(creatMsg)
   createMsg();
 
  int rc=0;
  assert(msgLen!=0);
  int remaining=msgLen;
  int total=0;
  while(total!=msgLen)
    {  
     rc=send(fd,buffer+total,remaining,MSG_NOSIGNAL);
     if(rc==-1)
     {
      if(errno==EPIPE)
	return 1;
      else
       {
        fprintf(stderr,"send from fd:%d failed:%s",fd,strerror(errno));
	return -1;
       }
     }
     remaining-=rc;
     total+=rc;
   }
    
    return 0;
}

int Protocol::recvAll(int fd)
{
 int rc;
 int total=0;
 int remaining=HEADERLEN;
 bool getBody=true; 

 getRest:

 while(remaining!=0)
 {
  rc=recv(fd,buffer+total,remaining,0);
  if(rc==-1)
   {
    fprintf(stderr,"recv from fd:%d failed:%s\n",fd,strerror(errno));
    return -1;
   }

  if(rc==0) return 1;

  total+=rc;
  remaining-=rc;
 }

 if(getBody)
 {
  remaining=parseHeader();
  
  if(remaining==-1)
   {
    fprintf(stderr,"recvAll(): parseHeader failed");
    return -1;
   }
   
  getBody=false;
  goto getRest;
 }
 return parseMsg();
}

char const * const Protocol::createMsg()
{
 
 unsigned short tracker=sizeof(Length);//skip MsgLen for now
 
 //copy the Id
 std::memcpy(buffer+tracker,&id,sizeof(id));
 tracker+=sizeof(id);
  
 std::memcpy(buffer+tracker,&protocolType,sizeof(protocolType));
 tracker+=sizeof(protocolType);
 
 std::memcpy(buffer+tracker,&code,sizeof(code));
 tracker+=sizeof(code);
 
 std::memcpy(buffer+tracker,&rqstNo,sizeof(rqstNo));
 tracker+=sizeof(rqstNo);
 
 std::memcpy(buffer+tracker,&rc,sizeof(rc));
 tracker+=sizeof(rc);

 int bodySize=createBody();
 if(bodySize==-1) assert(0);

 tracker+=bodySize;
 unsigned short tmp =htons(tracker);
 memcpy(buffer,&tmp,sizeof(short)); 
 msgLen=tracker;

 return buffer;
}

int Protocol::parseDataBlock(int&readSoFar)
{
  
  if(readSoFar+sizeof(Key)+sizeof(ValLen)>msgLen)
  {
   fprintf(stderr,"Could not Parse DataBlock as it exceeds the MsgLen\n");
   return -1;
  }
  
  std::memcpy(&dataBlock.key,buffer+readSoFar,sizeof(Key));
  dataBlock.key= ntohl(dataBlock.key);
  
  readSoFar+=sizeof(Key);
  
  std::memcpy(&dataBlock.valLen,buffer+readSoFar,sizeof(ValLen));
  dataBlock.valLen= ntohs(dataBlock.valLen);
  
  readSoFar+=sizeof(dataBlock.valLen);

  dataBlock.val=buffer+readSoFar;
  readSoFar+=dataBlock.valLen;
  
  if(dataBlock.valLen==0)
   dataBlock.val=NULL;

  return 0;
}

int Protocol::addDataBlock(unsigned short& writtenSoFar)
{ 
  if(writtenSoFar+sizeof(Key)+sizeof(ValLen)+dataBlock.valLen>MAX_PACKETLEN)
  {
   fprintf(stderr,"Could not add DataBlock as it exceeds the MAX_PACKETLEN\n");
   return -1;
  }
  
  Key tmpKey=htonl(dataBlock.key);
  ValLen tmpValLen= htons(dataBlock.valLen);

  std::memcpy(buffer+writtenSoFar,&tmpKey,sizeof(Key)); 
  writtenSoFar+=sizeof(Key);
  
  std::memcpy(buffer+writtenSoFar,&tmpValLen,sizeof(ValLen)); 
  writtenSoFar+=sizeof(dataBlock.valLen);

  std::memcpy(buffer+writtenSoFar,dataBlock.val,dataBlock.valLen); 
  writtenSoFar+=dataBlock.valLen;

 return 0;
}

void Protocol::setDataBlock(Key key, ValLen valLen,const char * val)
{
 dataBlock.key=key;
 dataBlock.valLen=valLen;
 dataBlock.val=val;
}

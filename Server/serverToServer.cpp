#include "serverToServer.h"

ServerToServer::ServerToServer()
{
 reset();
}

void ServerToServer::reset()
{
 Protocol::reset();
 allAddrs.clear();
 memset(&addr,'\0',sizeof(sockaddr_in)); 
 
 code=CODE_NONE;
 protocolType=SERVERTOSERVER;

 metaData.ds=0;
 metaData.version=0;
 metaData.ru=0;
}

int ServerToServer::parseMsg()
{ 
  int readSoFar=HEADERLEN; 
  fprintf(stderr,"Msg Received: Code:%d, Id:%d, sizeofMsg:%d\n",code,id,msgLen);
  if(code==CODE_ESTABLISHACK)
    return extractAllIpAddr(buffer,readSoFar,msgLen);
  
  else if(code==CODE_ESTABLISH)
   {
    if(readSoFar+SIZEOF_IPADDR_BYTE !=msgLen)
     return -1;
    
    Id lvId;
    getIPAddr(addr,buffer,readSoFar,lvId);
    return 0;
   }

   else if(code==CODE_INSERT || code==CODE_VOTE_REQUEST)
   {
    assert(parseDataBlock(readSoFar)==0);
    if(readSoFar!=msgLen)
      return -1;
    return 0;
   }
   else if(code==CODE_VOTE_REPLY || code==CODE_UPDATE)
   {
    assert(parseDataBlock(readSoFar)==0);
    assert(parseMetaBlock(readSoFar)==0);
    if(readSoFar!=msgLen)
	  return -1;
	return 0;
   }
   else if(code==CODE_GETMYID)
   {
    return 0;
   }

  else
   {
   fprintf(stderr,"Whats this code:%d\n",code);
   return -1;
  }
}

int ServerToServer::parseMetaBlock(int&readSoFar)
{
	  if(readSoFar+sizeof(DS)+sizeof(RU)+sizeof(Version)>msgLen)
	  {
	   fprintf(stderr,"Could not Parse MetaBlock as it exceeds the MsgLen\n");
	   return -1;
	  }

	  std::memcpy(&metaData.ru,buffer+readSoFar,sizeof(RU));
	  readSoFar+=sizeof(RU);

	  std::memcpy(&metaData.version,buffer+readSoFar,sizeof(Version));
	  readSoFar+=sizeof(Version);

	  std::memcpy(&metaData.ds,buffer+readSoFar,sizeof(DS));
	  readSoFar+=sizeof(DS);

	  return 0;
}

int ServerToServer::extractAllIpAddr(const char * const buffer, int fromLen, int totalLen)
{
 if(fromLen>=totalLen)
 {
  fprintf(stderr,"No Ip Addresses Included\n"); 
  return -1;
 }
 bool keepLooping=true;
 int nextLen=fromLen;
 struct sockaddr_in ip4addr;

 while(keepLooping)
  {
   nextLen+=SIZEOF_IPADDR_BYTE;
   if(nextLen<=totalLen)
    {
     Id p;
     getIPAddr(ip4addr,buffer,fromLen,p);
     allAddrs[p]=ip4addr;
     fromLen=nextLen;
    }
    else
     keepLooping=false;
  }

  if(fromLen!=totalLen)
    {
     fprintf(stderr,"fromLen not equal to end of Msg,currupted Msg");
     return -1;
    }

  return 0;
}

void ServerToServer::getIPAddr(struct  sockaddr_in& ip4addr,const char * const buffer,int fromLen,Id &p)
{ 
 
 ip4addr.sin_family=AF_INET;
 memcpy(&p,buffer+fromLen,sizeof(Id)); 
 memcpy(&ip4addr.sin_addr,buffer+fromLen+sizeof(p),sizeof(ip4addr.sin_addr)); 
 memcpy(&ip4addr.sin_port,buffer+fromLen+sizeof(p)+sizeof(ip4addr.sin_addr),sizeof(ip4addr.sin_port)); 
}

int ServerToServer::createBody()
{
 unsigned short tracker=HEADERLEN;
 
 Id p;
 sockaddr_in * ip4addr;
 unsigned short port;
 unsigned long addrToAdd;

 if(code==CODE_ESTABLISHACK)
 {
  for(auto& x:allAddrs)
   {
    p=x.first;
    ip4addr=&x.second;
    
    addrToAdd=ip4addr->sin_addr.s_addr;
    port=ip4addr->sin_port;
   
    memcpy(buffer+tracker,&p,sizeof(p));
    tracker+=sizeof(p);
    
    memcpy(buffer+tracker,&addrToAdd,sizeof(in_addr));
    tracker+=sizeof(in_addr);

    memcpy(buffer+tracker,&port,sizeof(short)); 
    tracker+=sizeof(short);
   }
 }

 if(code==CODE_ESTABLISH)
  {
    p=id;
    ip4addr=&addr;
    
    addrToAdd=(ip4addr->sin_addr.s_addr);
    port=(ip4addr->sin_port);
   
    memcpy(buffer+tracker,&p,sizeof(p));
    tracker+=sizeof(p);

    memcpy(buffer+tracker,&addrToAdd,sizeof(in_addr));
    tracker+=sizeof(in_addr);

    memcpy(buffer+tracker,&port,sizeof(short)); 
    tracker+=sizeof(short);
   }
   
if(code==CODE_INSERT || code==CODE_UPDATE || code==CODE_REPLY || code== CODE_VOTE_REQUEST || code== CODE_VOTE_REPLY)
  {
   assert(addDataBlock(tracker)==0);        
  }

if(code==CODE_VOTE_REPLY || code==CODE_UPDATE)
{
  assert(addMetaBlock(tracker)==0);
}
 
 return tracker-HEADERLEN;
}

void ServerToServer::printAllIPAddr(FILE * stream)
{

 fprintf(stream,"Printing All IP Addr Received\n");
 
 Id p;
 sockaddr_in * ip4addr;
 unsigned short port;
 unsigned long addrToAdd;
 char ipstr[INET_ADDRSTRLEN];

 for(auto& x:allAddrs)
 {
  p=x.first;
  ip4addr=&x.second;
  port=ip4addr->sin_port;
  inet_ntop(ip4addr->sin_family,&(ip4addr->sin_addr),ipstr,sizeof ipstr);
  fprintf(stream,"PROCESS:%d IP:%s:%d\n",p,ipstr,ntohs(port));
 }

}

int ServerToServer::addMetaBlock(unsigned short& writtenSoFar)
{
  if(writtenSoFar+sizeof(RU)+sizeof(DS)+sizeof(Version)+dataBlock.valLen>MAX_PACKETLEN)
  {
   fprintf(stderr,"Could not add DataBlock as it exceeds the MAX_PACKETLEN\n");
   return -1;
  }

  std::memcpy(buffer+writtenSoFar,&metaData.ru,sizeof(RU));
  writtenSoFar+=sizeof(RU);

  std::memcpy(buffer+writtenSoFar,&metaData.version,sizeof(Version));
  writtenSoFar+=sizeof(Version);

  std::memcpy(buffer+writtenSoFar,&metaData.ds,sizeof(DS));
  writtenSoFar+=sizeof(DS);

 return 0;
}

#include "clientToServer.h"

ClientToServer::ClientToServer()
{
 reset();
}

void ClientToServer::reset()
{
 Protocol::reset();
 code=CODE_NONE;
 protocolType=CLIENTTOSERVER; 
 serverNames.clear();
}


int ClientToServer::parseMsg()
{ 
  int readSoFar=HEADERLEN; 
  fprintf(stderr,"Msg Received: Code:%d, Id:%d, sizeofMsg:%d\n",code,id,msgLen); 
  
  
if(code==CODE_REPLY || code==CODE_UPDATE || code==CODE_INSERT || code==CODE_READ || code==CODE_DELETE)
   {
    assert(parseDataBlock(readSoFar)==0);

    
    if(code==CODE_UPDATE || code==CODE_INSERT)
    {
     unsigned char numServers=0;
     ServerName serverName=0;

     std::memcpy(&numServers,buffer+readSoFar,sizeof(numServers));
     readSoFar+=sizeof(numServers);

     for(int i=0;i<numServers;i++)
      {
      std::memcpy(&serverName,buffer+readSoFar,sizeof(ServerName));
      readSoFar+=sizeof(ServerName);
      serverNames.push_back(serverName);
      }
   }
    
    if(readSoFar!=msgLen)
      {
      fprintf(stderr,"Error with Length of Message\n");
      serverNames.clear();
      return -1;
      }

   return 0;
 }
  fprintf(stderr,"Whats this code:%d\n",code);
  return -1;
}

int ClientToServer::createBody()
{
 unsigned short tracker=HEADERLEN;
 
 if(code==CODE_REPLY || code==CODE_UPDATE || code==CODE_INSERT || code==CODE_READ || code==CODE_DELETE)
   { 
    assert(addDataBlock(tracker)==0);
    
    if(code==CODE_UPDATE || code==CODE_INSERT)
    {
     unsigned char numServers=serverNames.size();
     ServerName serverName=0;

     std::memcpy(buffer+tracker,&numServers,sizeof(numServers));
     tracker+=sizeof(numServers);

     for(int i=0;i<numServers;i++)
      {
      serverName=serverNames[i];
      std::memcpy(buffer+tracker,&serverName,sizeof(ServerName));
      tracker+=sizeof(ServerName);
      } 
     }
   }
 else 
 {
  fprintf(stderr,"CreateBody, What code is this?? Code:%d",code); 
  serverNames.clear();
  assert(0);
 }
 
 return tracker-HEADERLEN;	 
}

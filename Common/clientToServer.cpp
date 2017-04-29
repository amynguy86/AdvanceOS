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
}


int ClientToServer::parseMsg()
{ 
  int readSoFar=HEADERLEN; 
  fprintf(stderr,"Msg Received: Code:%d, Id:%d, sizeofMsg:%d\n",code,id,msgLen); 
  
  
if(code==CODE_REPLY || code==CODE_UPDATE || code==CODE_INSERT || code==CODE_READ || code==CODE_DELETE)
   {
    assert(parseDataBlock(readSoFar)==0);

    if(readSoFar!=msgLen)
      {
      fprintf(stderr,"Error with Length of Message\n");
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
   }
 else 
 {
  fprintf(stderr,"CreateBody, What code is this?? Code:%d",code); 
  assert(0);
 }
 
 return tracker-HEADERLEN;	 
}

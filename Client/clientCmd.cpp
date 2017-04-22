/*
 * Client Command Line too issue requests to servers 
 */

//#define UTD
#include "../Common/clientToServer.h"
#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <mutex>
#include <functional>
#include <string>
#include <stdlib.h>

#define  PORT 1025 //server hearing on this port for UTD, for cygwin starting port number
#define NUMSERVERS 4
using namespace std;

/*
 * All Server IP Addrs
 * hard codeded here
*/

#ifdef UTD
std::vector<string> serverIps={
"10.176.66.51",
"10.176.66.52",
"10.176.66.53",
"10.176.66.54",
"10.176.66.55",
"10.176.66.56",
"10.176.66.57",
};

//Port number of the individual servers
std::vector<int> serverPorts={
PORT,
PORT,
PORT,
PORT,
PORT,
PORT,
PORT,
};
#else
std::vector<string> serverIps={
"16.99.130.183",
"16.99.130.183",
"16.99.130.183",
"16.99.130.183",
"16.99.130.183",
"16.99.130.183",
"16.99.130.183",
};

//Port number of the individual servers
std::vector<int> serverPorts={
PORT+0,
PORT+1,
PORT+2,
PORT+3,
PORT+4,
PORT+5,
PORT+6,
};
#endif

static std::mutex wait;
static std::unordered_map<Id,int> processFDMap; //needs to be protected
static std::hash<Key> key_hash;
char port[50];

void fnExit(void)
{
cout<<"Exiting"<<endl;
}

typedef struct 
{
 int fd;
 int serverNo;
}Data;

static void receiver(Data data)
{
 ClientToServer rqstMsg;
 
 while(true)
 {
  int rc=rqstMsg.recvAll(data.fd);
  if(rc!=0)
   {
    close(data.fd);
    processFDMap[data.serverNo]=-1;
    return; //connection lost
   } 
   if(rqstMsg.getRc()==RC_SUCCESS)
   {
    cout<<"Success(ServerNo:"<<(int)rqstMsg.getId()<<")"<<endl;
    if(rqstMsg.getValLen())
     cout<<"Value:"<<rqstMsg.getVal()<<endl;
   }
   else
   {
    cout<<"Fail"<<endl;
   }
   wait.unlock();
 }
}

int main(int argc,char *argv[])
{
 int status;
 struct addrinfo hints;
 struct addrinfo *servinfo;
 void * addr;
 int sockFd;

 memset(&hints,0,sizeof(hints));
 hints.ai_family=AF_INET;
 hints.ai_socktype =SOCK_STREAM;
 hints.ai_flags = AI_PASSIVE;
 
 //Connect to All servers
 for(int i=0;i<NUMSERVERS;i++)
 {
  int status;
  snprintf(port, sizeof(port), "%d", serverPorts[i]);

  if ((status = getaddrinfo(serverIps[i].c_str(),port, &hints, &servinfo)) != 0)
  {
   fprintf(stderr, "getaddrinfo error: %s for IP:%s\n", gai_strerror(status),serverIps[i].c_str());
   return -1;
  }
      
 sockFd = socket(PF_INET,SOCK_STREAM,0);
 
 if (sockFd==-1)
 {
  perror("socket");
  return -1;
 }
  if(connect(sockFd,servinfo->ai_addr,servinfo->ai_addrlen)==-1)
   {
      perror("Connect");
      close(sockFd);
      return -1;
   }

 //start reciever
 processFDMap[i]=sockFd;
 Data data;
 data.fd=sockFd;
 data.serverNo=i;
 thread * newThread=  new thread(receiver,data);
 }

 string request;
 string value;
 Key key;
 ClientToServer rqstMsg;
 unsigned char rqstNo=0;
 wait.lock();
 
 cout<<"Ready to take request"<<endl;
 while(true)
 {

  rqstMsg.reset();
  cout<<"-->";
  getline(std::cin,request);
 
  std::stringstream ss(request);
  std::string token;
  ss>>token;
  if(token.length()!=1){cout<<"Wrong Command"<<endl; continue;}
  switch(token[0])
  {
   case 'R':
    ss>>key;
    rqstMsg.setCode(CODE_READ);
    rqstMsg.setDataBlock(key);
    rqstMsg.setRc(RC_NONE);
    rqstMsg.setId(1); //does not matter
    rqstMsg.setRqstNo(rqstNo);
    ss>>key;	
    if(rqstMsg.sendAll(processFDMap[key])==0)
      wait.lock();
    else
     {
     cout<<"Unable to send Request:Server Not Availaible\n"<<endl;
     }
   break;

   case 'U':
   case 'I':
    {
    if(token[0]=='I')
     rqstMsg.setCode(CODE_INSERT);
    else
     rqstMsg.setCode(CODE_UPDATE);
    
    ss>>key;
    ss>>value;

    std::vector<ServerName> numServers;
    
    for(int i=0;i<3;i++)
     {
     try{
      if(processFDMap.at((((key_hash(key))%7)+i)%7)!=-1)
       numServers.push_back((((key_hash(key))%7)+i)%7);
     }
     catch(const std::out_of_range& oor)
     {
      cout<<"Caught"<<endl; 
     }
      cout<<"Hashed to:"<<(((key_hash(key))%7)+i)%7<<endl;
     }

    if(numServers.size()<2)
     {
      if(numServers.size()==1)
       cout<<"only one server is UP and that is:"<<(int)numServers[0]<<endl;
      else
       cout<<"No Server is up to serve the Request"<<endl;
      continue;
     }
    else
     {
      for(int i=1;i<numServers.size();i++)
       {
        rqstMsg.addServer(numServers[i]);
       }
     }

    rqstMsg.setDataBlock(key,value.length(),value.c_str());
    rqstMsg.setRc(RC_NONE);
    rqstMsg.setId(1); //does not mat/ter
    rqstMsg.setRqstNo(rqstNo);
    

    if(rqstMsg.sendAll(processFDMap[numServers[0]])==0)
      wait.lock();
    else
     {
     cout<<"Unable to send Request\n"<<endl;
     }
   }
   break;
   
   case 'H':  
   ss>>key;
   cout<<"Hashes To:"<<endl;
   for(int i=0;i<3;i++)
    cout<<(((key_hash(key))%7)+i)%7<<endl;
   break;
  
  case 'B':
  {
  ss>>key;
  int fd=processFDMap[key];
  processFDMap[key]=-1;
  close(fd);
  cout<<"Broke Connection with server:"<<(int)key<<endl;
  }
  break;
  
  default:
    cout<<"Wrong Command"<<endl;
  }

  rqstNo++;
 }
 return 0; 
}


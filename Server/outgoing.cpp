#include "outgoing.h"
int Outgoing::tearDown()
{
 if(processNum==0)
  {
   toreDown=true;
   return 0;
  }
 //send Compelte Notification to   master Process
 cout<<"Sending Complete Notification to master process"<<endl;
 sendMsg.reset();
 sendMsg.setId(processNum);
 sendMsg.setCode(CODE_COMPLETE);
 sendMsg.setRc(RC_SUCCESS);
 sendMsg.setRqstNo(255); //just some random no
 toreDown=true;
 
 assert(sendMsg.sendAll(processFDMap[0].fd)==0);
 return 0;
}
int Outgoing::establishConnWithMain()
{ 

 if(processNum==0)
  {
  //all Connections for the main process are established in the Incoming
  
  /*
   * Below code was moved to setProcessNumber because there could be a situatuin where
   * mutex is locked after mutex is unlocked by Incoming thread;
   */

  //ivMut.lock(); //have to wait for Incoming to establish connection with all other processes 
  return 0; //main process does not establish connection
  }
 
 int sockFd = socket(PF_INET,SOCK_STREAM,0);
 if (sockFd==-1)
 {
  perror("establishConn");
  return -1;
 }
 if(connect(sockFd,(struct sockaddr *)&mainProcAddr,sizeof mainProcAddr)==-1)
  {
  perror("Connect");
  close(sockFd);
  return -1;
  }
 
 /*
 add Main Proc FD Into Hash
 This hash does not need to protected with a mutex because it has several reader but only one writer
 Writing takes place either here for slave processes or in Incoming for master  
 */
 processFDMap[0].fd=sockFd;

 //todo check if these values have been set
 sendMsg.setCode(CODE_ESTABLISH);
 sendMsg.setId(processNum);
 sendMsg.setMyAddr(myAddr);
 sendMsg.setRc(RC_NONE);
 sendMsg.setRqstNo(1); //this does not apply here

 int rc=0;
 rc= sendMsg.sendAll(sockFd);
 
 if(rc!=0)
  return -1;
 
 cerr<<"Succesfully Sent Establish"<<endl;
  
 msgParser.reset();
 rc=msgParser.recvAll(sockFd); 
 
 if(rc==-1)
  {
   cerr<<"recvALL was not succesfull"<<endl;
   return -1;
  } 
else if(rc==1)
 {
 cerr<<"Connection broken by Host"<<endl;
 return -1;
 }

 cerr<<"Succesfully Recieved all IP Addrs, Size:"<<msgParser.getMsgLen()<<endl;
 msgParser.printAllIPAddr(stderr); 
 return 0;
}

void Outgoing::addToProcessFDMap(Id processNum,int fd)
{
 processFDMap[processNum].fd=fd;
 processFDMap[processNum].enabled=true;

 if(processFDMap.size()>totalProc-1)
 {
  cerr<<"All Processes Have been already addded to the map"<<endl;
  processFDMap.erase(processNum);
  return;
 }

 if(processFDMap.size()==totalProc-1)
  ivMut.unlock(); //all 
}

int Outgoing::establishConnWithAll()
{

//poll Wait for Mutex on  ProcessFdMap Size for Main Process 0
if(processNum==0)
 {
 ivMut.lock();
 ivMut.unlock();
 
 std::cerr<<"MasterProc:Successfully established Connection with All processes"<<std::endl;
 return 0;
 }

for(auto& x : *msgParser.getAllIpAddr())
 {
 
 if(x.first==processNum) continue;
 
 int sockFd = socket(PF_INET,SOCK_STREAM,0);
 
 if (sockFd==-1)
 {
  perror("establishConn");
  return -1;
 }

 if(connect(sockFd,(struct sockaddr *)&x.second,sizeof x.second)==-1)
  {
  perror("Connect");
  close(sockFd);
  return -1;
  }

  //send first Msg
  ServerToServer reqMsgToServer;
  reqMsgToServer.setId(processNum);
  reqMsgToServer.setRc(RC_SUCCESS);
  reqMsgToServer.setCode(CODE_GETMYID);
  reqMsgToServer.setRqstNo(0); //does not matter
  reqMsgToServer.sendAll(sockFd);

  processFDMap[x.first].fd=sockFd;
 }

 std::cerr<<"Successfully established Connection with All processes"<<std::endl;

 return 0;
}


int Outgoing::send(Id toProcessNum,ServerToServer &reqMsgToServer)
{
  if(isLinkDisabled(toProcessNum)) return -1;

  assert(reqMsgToServer.sendAll(processFDMap[toProcessNum].fd)==0);
  return 0;
}

int Outgoing::sendMsgToServers(const vector<Id> &serverToSend,ServerToServer reqMsgToServer)
{
 for(Id x:serverToSend)
 {
  if(send(x,reqMsgToServer)!=0)
	  return -1;
 }
 return 0;
}

void Outgoing::stop()
{
 ivMut.try_lock();   
 ivMut.unlock(); //incase something is stuck here in the event computation could not progress due to some error
 for(auto& x:processFDMap)
  {
  shutdown(x.second.fd,2);
  close(x.second.fd);
  }
}

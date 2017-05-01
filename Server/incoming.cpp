#include "incoming.h"
std::mutex Incoming::ivMut; //mutex
std::condition_variable Incoming::ivCond;

void Incoming::joinAllThreads()
{
 for(auto& x:myThread_fds)
  {
   x.myThread->join();
   delete x.myThread;
  }
}

int Incoming::bindToIp(const char * const myIp,const char * const port,bool isForClient)
{
 int status;
 struct addrinfo hints;
 struct addrinfo *servinfo;
 char ipstr[INET_ADDRSTRLEN];
 void * addr;

 memset(&hints,0,sizeof(hints));
 hints.ai_family=AF_INET;
 hints.ai_socktype =SOCK_STREAM;
 hints.ai_flags = AI_PASSIVE;
 
 int * fd;
 if(!isForClient)
  fd = &serverFd;
 else
  fd =&clientFd;

 string ip;
 
 if(myIp==NULL)
   ip ="0.0.0.0";
 else
   ip=myIp; 

 if ((status = getaddrinfo(ip.c_str(), port, &hints, &servinfo)) != 0)
  {
   fprintf(stderr, "getaddrinfo error: %s for IP:%s\n", gai_strerror(status),ip.c_str());
    return -1;
  }
         
 *fd=socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
     
 if (*fd == -1)
 {
  fprintf(stderr, "socket error: %d\n",errno );
  return -1;
 }
 int enable=1;
 if(setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
   {
   perror("setsockopt(SO_REUSEADDR) failed");
   return -1;
   }

 if(bind(*fd,servinfo->ai_addr,servinfo->ai_addrlen)==-1)
  {
   perror("Bind");
   return-1;
  }
 
 if(!isForClient)
 {
  socklen_t size= sizeof(struct sockaddr_in);
  getsockname(*fd,(struct sockaddr *)&myIP,&size);
  fprintf(stderr,"Successfully Bounded for server on %s:%d\n", inet_ntoa(myIP.sin_addr),ntohs(myIP.sin_port));
 }
 else
 {
  fprintf(stderr,"Successfully Bounded for client on %s\n",myIp);
 }
 return *fd;
}

int Incoming::start()
{
 if(serverFd==-1)
   return -1;

 if(listen(serverFd,BACKLOG)==-1)
  {
   perror("listen");
   return -1;
  }
 
  fprintf(stderr,"listening\n");
  stopped=false;

  return waitForConn();
}

int Incoming::allowClientRqsts()
{
 int newFd;
 if(clientFd==-1)
   return -1;

 struct sockaddr_in their_addr;
 socklen_t sin_size=sizeof their_addr;

 if(listen(clientFd,BACKLOG)==-1)
  {
   perror("listen");
   return -1;
  }
 
  fprintf(stderr,"listening on Client Fd\n");
  fprintf(stderr,"Server: Waiting for Connections to server Clients\n");
  
  ClientToServer reqMsgFromClient;

  while(!stopped)
  {
   newFd=accept(clientFd, (struct sockaddr *)&their_addr,&sin_size);

   if(newFd==-1)
    {
     perror("accept");
     return -1;
    }
  
  Obj_method obj_method;
  obj_method.obj=this;
  obj_method.fd=newFd;
     
 thread * newThread = new thread(serveClientRqsts,obj_method);

  }
  return 0 ;
}

void Incoming::serveClientRqsts(Obj_method obj_method)
{
 int fromFd=obj_method.fd;
 ServerToServer reqMsgToServer;
 ClientToServer replyMsgToClient; 
 ClientToServer reqMsgFromClient;
 std::vector<Id> serversToSend;

 while(true)
 {
 serversToSend.clear();
 reqMsgToServer.reset();
 replyMsgToClient.reset();
 reqMsgFromClient.reset();

 int rc=reqMsgFromClient.recvAll(fromFd);
 if(rc==1){close(fromFd); return;}

 if(rc!=0){close(fromFd); return;}
 
 fprintf(stderr,"Requst recieve from clientFd:%d",reqMsgFromClient.getId());
 fprintf(stderr,"Code:%d ,Key:%d ValLen:%d",reqMsgFromClient.getCode(),reqMsgFromClient.getKey(),reqMsgFromClient.getValLen());
 bool isUpdate=false;

 switch(reqMsgFromClient.getCode())
 {
  case CODE_READ:
   {
   const char * data=Memory::getInstance()->read(reqMsgFromClient.getKey());
   if(data==NULL)
    {
    replyMsgToClient.setRc(RC_FAIL);
    replyMsgToClient.setDataBlock(reqMsgFromClient.getKey());
    }
   else
    {
    replyMsgToClient.setRc(RC_SUCCESS);
    replyMsgToClient.setDataBlock(reqMsgFromClient.getKey(),strlen(data),data);
    }
    
    replyMsgToClient.setId(obj_method.obj->myProcNum);
    replyMsgToClient.setCode(CODE_REPLY);
    replyMsgToClient.setRqstNo(reqMsgFromClient.getRqstNo());
    replyMsgToClient.sendAll(fromFd);
    break;
   }

  //CODE_INSERT is also not applicable to Project3
  case CODE_UPDATE:
  case CODE_INSERT:
  {
  if(reqMsgFromClient.getCode()==CODE_INSERT)
  {
   isUpdate=false;
   assert(0);
  }
  else
   isUpdate=true;

  unsigned char serverToWaitOn=0;
  unsigned char serverBit=1;
  
  /*
   * Amin: Once a server receives a request, it should forward vote_request to all
   * other servers that are in the cluster
   */
   reqMsgToServer.setRc(RC_NONE);
   reqMsgToServer.setId(obj_method.obj->myProcNum);
   reqMsgToServer.setCode(CODE_VOTE_REQUEST);
   reqMsgToServer.setRqstNo(reqMsgFromClient.getRqstNo());
   reqMsgToServer.setDataBlock(reqMsgFromClient.getKey());

   int serverFd=-1;
   int rc;

 //construct the servers to wait on(those that are alive)
 serverToWaitOn=obj_method.obj->outgoingHandler->getServersToWaitOn(serversToSend);

 rc=Memory::getInstance()->AddOrUpdateWithLock(reqMsgFromClient.getKey(),reqMsgFromClient.getVal(),fromFd,obj_method.obj->myProcNum,serverToWaitOn,reqMsgFromClient.getRqstNo(),isUpdate);

 if(rc!=0)
  {
	Memory::getInstance()->forceUnlockData(reqMsgFromClient.getKey());
    replyMsgToClient.setRc(RC_FAIL);
    replyMsgToClient.setDataBlock(reqMsgFromClient.getKey());
    replyMsgToClient.setId(obj_method.obj->myProcNum);
    replyMsgToClient.setCode(CODE_REPLY);
    replyMsgToClient.setRqstNo(reqMsgFromClient.getRqstNo());
    replyMsgToClient.sendAll(fromFd);
  }
 else
 {
	 if(obj_method.obj->outgoingHandler->sendMsgToServers(serversToSend,reqMsgToServer)!=0)
	 {
		 Memory::getInstance()->forceUnlockData(reqMsgFromClient.getKey());
		 replyMsgToClient.setRc(RC_FAIL);
		 replyMsgToClient.setDataBlock(reqMsgFromClient.getKey());
		 replyMsgToClient.setId(obj_method.obj->myProcNum);
		 replyMsgToClient.setCode(CODE_REPLY);
		 replyMsgToClient.setRqstNo(reqMsgFromClient.getRqstNo());
		 replyMsgToClient.sendAll(fromFd);
	 }
 }

  break;
  }
 default:
  assert(0);
 }
 }//end while
}
void Incoming::stop()
{ 
 if(stopped) return;
 stopped=true;
 outgoingHandler->stop();

 if(serverFd!=-1)
  {
   shutdown(serverFd,2);
   close(serverFd);
  }
 
 if(clientFd!=-1)
  {
   shutdown(clientFd,2);
   close(clientFd);
  }

 for(auto& x:myThread_fds)
  {
   shutdown(x.fd,2);
  }
  
  ivCond.notify_all();
  
}

Incoming::~Incoming()
{
 stop();
}

int Incoming::waitForConn()
{
 int newFd;
 newFd=-1;

 struct sockaddr_in their_addr;
 socklen_t sin_size=sizeof their_addr;
 

 while(!stopped && myThread_fds.size()!=totalProc-1)
 {
  fprintf(stderr,"Server: Waiting for Connections\n");
  newFd=accept(serverFd, (struct sockaddr *)&their_addr,&sin_size);

  if(newFd==-1)
   {
    perror("accept");
    return -1;
   }
 Obj_method obj_method;
 obj_method.obj=this;
 obj_method.fd=newFd;
 thread * newThread=  new thread(communicate,obj_method);
 
 Thread_fd thread_fd;

 thread_fd.myThread=newThread;
 thread_fd.fd=newFd;
 
 myThread_fds.push_back(thread_fd);
 }

 if(totalProc-1==myThread_fds.size())
  {
   cerr<<"Got Connection from Every Other Server"<<endl;
   return 0;
  }
 return -1;
}

void Incoming::communicate(Obj_method obj_method)
{
 ServerToServer reqMsg;
 int rc;
 Id fromProc;
 Id myProc;
 int totalProc;
 ServerToServer replyMsg;
 ClientToServer replyMsgToClient;
 Outgoing * outgoingHandler=obj_method.obj->getOutgoingHandler();
 bool established=false;
 bool firstMsgRcvd=false;
 bool isUpdate=false;
 loop:
 reqMsg.reset();
 replyMsg.reset();
 replyMsgToClient.reset();
 rc=reqMsg.recvAll(obj_method.fd);
 


if(rc==-1)
   assert(0);
 
 if(rc==1)
  {
  if(!firstMsgRcvd)
   assert(0);

  if(!obj_method.obj->stopped)
   {
    //todo in the future we can try to reestablish connection
    fprintf(stderr,"Server %d is closing connection,firstMsgRecived is true\n",fromProc);
   }
  else
   {
   cerr<<"Recv fd closed by this process"<<endl;
   }
   
   close(obj_method.fd);
   return;
 }
 
 assert((firstMsgRcvd && fromProc==reqMsg.getId()|| !firstMsgRcvd)); 
 
 if(!firstMsgRcvd)
  fromProc=reqMsg.getId();
 
 totalProc = obj_method.obj->getTotalProc();
 myProc = obj_method.obj->getMyProcNum();
 
 tryAgain:
 switch(reqMsg.getCode())
  {
   case CODE_ESTABLISH:
    {
      assert(fromProc!=0 && myProc==0 && !established); //0 does not sent ESTABLISH, it receives it(one establish per connection)
       int sockFd = socket(PF_INET,SOCK_STREAM,0);
       
       if (sockFd==-1)
	{
	  perror("Incomming::Communicate");
	  sleep(1);
	  goto tryAgain;
	}
     
      if(connect(sockFd,(struct sockaddr *)&reqMsg.getAddr(),sizeof(reqMsg.getAddr()))==-1)
	 {
	  perror("Incoming::Communicate");
          close(sockFd);
          sleep(1);
	  goto tryAgain;
	}

    //just a ok I am here sg
     replyMsg.setId(myProc);
     replyMsg.setRc(RC_SUCCESS);
     replyMsg.setCode(CODE_GETMYID);
     replyMsg.setRqstNo(0); //does not matter
     replyMsg.sendAll(sockFd);
           

      {
      std::unique_lock<std::mutex> lck(ivMut);
      //todo once a connection is established, this should not happen
      outgoingHandler->addToProcessFDMap(fromProc,sockFd); //write to procesFDMap one thread at a time
      ProcessIPMap &tmp=*(obj_method.obj->getProcessIPMap());
      tmp[reqMsg.getId()]=reqMsg.getAddr();
      
      while (obj_method.obj->getProcessIPMap()->size()!=totalProc-1 && !obj_method.obj->stopped)
      {
       ivCond.wait(lck);
      }
      ivCond.notify_all();

      //send all IP Addrs
      cerr<<"Sending All IpAddrs to Process No"<<(int)fromProc<<endl;
      replyMsg.setCode(CODE_ESTABLISHACK);
      replyMsg.setId(myProc);
      replyMsg.setRc(RC_NONE);
      replyMsg.setRqstNo(0);

      replyMsg.setAllIpAddr(tmp); 
      }
      assert(replyMsg.sendAll(obj_method.fd)==0);
      established=true;
      break;
    }
    
   case CODE_GETMYID:
   { //this will only be recieved by everyone except the master process to associate this fd with the process number 
    fromProc=reqMsg.getId();
   break;
   }
   
   case CODE_INSERT:
   case CODE_UPDATE:
   { 
    if(reqMsg.getCode()==CODE_INSERT)
    {
      //should not occur for project 3
      assert(0);
      isUpdate=false;
    }
   else 
      isUpdate=true;
    
    int rc=Memory::getInstance()->AddOrUpdate(reqMsg.getKey(),reqMsg.getVal(),reqMsg.getMetaData(),isUpdate);
    if(rc!=0)
      assert(0);
    break;
   }

   case CODE_VOTE_REPLY:
   {
    Memory::Holder::MetaData metaData;
    std::string val;
    unsigned char serverBit=1;
    serverBit= serverBit<<reqMsg.getId();
    if(reqMsg.getRc()!=RC_SUCCESS){fprintf(stderr, "the server No:%d, return unsuccessful result Code",reqMsg.getId()); assert(0);}
    
    fprintf(stderr,"MeteData From Server%d: RU:%d,Version:%d,DS:%d\n",reqMsg.getId(),reqMsg.getMetaData().ru,reqMsg.getMetaData().version,reqMsg.getMetaData().ds);
    int rc=Memory::getInstance()->unlockData(reqMsg.getKey(),reqMsg.getId(),val,reqMsg.getRqstNo(),serverBit,reqMsg.getMetaData(),metaData);
    
    if(rc==-1) assert(0); //todo not sure what to do here
    else if(rc==1) break;
    else if(rc==2)
    {
	//unable to form majority(todo code Missing in Memory)
	replyMsgToClient.setRc(RC_FAIL);
	replyMsgToClient.setId(myProc);
	replyMsgToClient.setCode(CODE_REPLY);
	replyMsgToClient.setRqstNo(metaData.rqstNo);
	replyMsgToClient.sendAll(metaData.initiatingClient);
	break;
    }
    
   //succesfully got Majority so update all servers
   for(int i=0;i<sizeof(unsigned char)*8;i++)
   {
	   if(metaData.serverWaitingOnUnchanged & 1<<i)
	   {
	   replyMsg.setRc(RC_NONE);
	   replyMsg.setId(obj_method.obj->myProcNum);
	   replyMsg.setCode(CODE_UPDATE);
	   replyMsg.setRqstNo(reqMsg.getRqstNo());
	   replyMsg.setDataBlock(reqMsg.getKey(),val.size(),val.c_str());//data form memory)
	   replyMsg.setMetaData(metaData.ru,metaData.version,metaData.ds);
	   assert(outgoingHandler->send(i,replyMsg)==0);
	   }
   }

    replyMsgToClient.setRc(RC_SUCCESS);
    replyMsgToClient.setId(myProc);
    replyMsgToClient.setCode(CODE_REPLY);
    replyMsgToClient.setRqstNo(metaData.rqstNo);
    replyMsgToClient.sendAll(metaData.initiatingClient);

    break;
   }

   case CODE_VOTE_REQUEST:
   {
	   RU ru=0;
	   DS ds=0;
	   Version v=0;

	   int rc=Memory::getInstance()->readMetaData(reqMsg.getKey(),ru,v,ds);

	   replyMsg.setId(myProc);
	   replyMsg.setCode(CODE_VOTE_REPLY);
	   replyMsg.setRqstNo(reqMsg.getRqstNo());
	   replyMsg.setMetaData(ru,v,ds);
	   replyMsg.setDataBlock(reqMsg.getKey());

	   if(rc==0)
	   {
	   replyMsg.setRc(RC_SUCCESS);
	   }
	   else
	   {
	   replyMsg.setRc(RC_FAIL);
	   }

	   assert(outgoingHandler->send(reqMsg.getId(),replyMsg)==0);

	   break;
   }

   default:
    assert(0);
 }
  firstMsgRcvd=true;
  goto loop;
}


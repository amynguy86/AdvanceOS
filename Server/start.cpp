/*
 * Server.cpp 
 * This is the Server. It communicates with other server 
 * via the incoming and outgoing threads.
 *
 * The incoming and outgoing threads allow the server to receive requests
 * from other servers as well as make requests to other servers. That is
 * it acts a server as well as a client to other servers.
 *
 * The master server 0 needs to be started first as it coordinates exchange of
 * IP addresses amoung all other servers
 *
 * Ideally, I would have put all this is in a "ServerClass" as it deals with Server To Server Communication.
 * have another file called "ClientServerClass" by which the server deals with clients
 * However at this point code that deals with Client/Server communication is also inside Incoming/Outgoing.
 *
 */

#include "outgoing.h"
#include "incoming.h"
#include <thread>
#include <vector>
static string  PORT ="1025"; //use for hearing client msgs
#define PORT_MASTER_SERVER "1024" //used for  master server

using namespace std;

/*
 * Shared Database 
 */ 

static int serverFd=-1;
static int clientFd=-1;
static Incoming incoming;
static Outgoing outgoing;
static std::mutex doneMut;
static std::condition_variable doneCond;  
static bool connected_to_everyone=false;
static bool everyone_connected=false;
static bool incomingDone=false;
static bool outgoingDone=false;
static int ME;
static int N;

void fnExit(void)
{
cout<<"Exiting"<<endl;
}

static void checkStop()
{
 string stop="";
 while(true)
 {
  cin>>stop;
 if(stop=="stop")
  {
   incoming.stop();
   break;
  }
 else
  cout<<"Could Not Understand Command, Type stop to exit"<<endl;
 }
}

static void runIncoming()
{
int rc=incoming.start();
if(rc==0) //notify Application
 everyone_connected=true;

incomingDone=true;
doneCond.notify_all();
incoming.joinAllThreads();
}

static void runOutgoing()
{
if(outgoing.establishConnWithMain()!=-1);
	if(outgoing.establishConnWithAll()!=-1);
		connected_to_everyone=true;

outgoingDone=true;
doneCond.notify_all();
}


int main(int argc,char *argv[])
{
atexit(fnExit);

char * myIp=NULL;

string ipaddrMasterProc;


if(argc<4|| argc>6)
{

  cout<<"This program starts the Server"<<endl;
  cout<<"This program expects atleast four arguments"<<endl;
  cout<<"	1. Id of this server(0-6)"<<endl;
  cout<<"	2. Total Number of servers in the distributed system"<<endl;
  cout<<"	3. Ip Adress and Port of the master Server 0. eg. 192.168.1.1:7770. For Master process just provide anything"<<endl;
  cout<<"        4. Ip Address to bind to for communication(will pick port 1024 for Master Server always)"<<endl;
  cout<<"        5. Port to Listen for ServertoClient communication(Optional-Default is 1025)"<<endl;
  cout<<"Example:./runServer 0 4 129.110.92.21:0000 129.110.92.21 1025 2>/dev/null"<<endl;
  
 return 0;
}

start:
ME = atoi(argv[1]);
N = atoi(argv[2]);
ipaddrMasterProc= argv[3];
int portIndex=ipaddrMasterProc.find(':');

if(portIndex==-1)
{
 cout<<"No port number is masterprocaddr"<<endl;
 return 0;
}
struct sockaddr_in masterProcAddr={0};

if(inet_pton(AF_INET,ipaddrMasterProc.substr(0,portIndex).c_str(),&(masterProcAddr.sin_addr))!=1)
  {
  cout<<"invalid MasterProcAddr"<<endl;
  return 0;
  }
masterProcAddr.sin_family=AF_INET;
masterProcAddr.sin_port=htons(atoi(ipaddrMasterProc.substr(portIndex+1).c_str()));
cout<<"Master Proc Addr"<<ipaddrMasterProc.substr(0,portIndex)<<"Port:"<<atoi(ipaddrMasterProc.substr(portIndex+1).c_str())<<endl;
myIp=argv[4];

incoming.setTotalProc(N);
incoming.setOutgoingHandler(&outgoing);
incoming.setMyProcNum(ME);
if(ME==0)
{
	if(incoming.bindToIp(myIp,PORT_MASTER_SERVER,false)==-1) return 0;
}
else
{
	if(incoming.bindToIp(myIp,NULL,false)==-1) return 0;
}

outgoing.setProcessNum(ME);
outgoing.setMyAddr(incoming.getMyIP());
outgoing.setMainProcAddr(masterProcAddr);
outgoing.setTotalProc(N);


std::thread incomingThread(runIncoming);
std::thread outgoingThread(runOutgoing);
std::thread stopThread(checkStop);

{
 std::unique_lock<std::mutex> lck(doneMut);

 while(!outgoingDone || !incomingDone)
 {
  doneCond.wait(lck);
 }
 
 if(connected_to_everyone && everyone_connected)
 {
  if(argc==6)
	  PORT=argv[5];
  if(incoming.bindToIp(myIp,PORT.c_str(),true)==-1) return 0;
  cout<<"Ready To Take Requests From Client"<<endl;
  if(incoming.allowClientRqsts()==-1) return 0;
 }
 else
  cout<<"Unable to Start Application"<<endl;
}
incomingThread.join();
outgoingThread.join();
stopThread.join();
}


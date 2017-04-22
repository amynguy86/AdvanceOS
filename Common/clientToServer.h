/* This is the protocol utlized for Client to Server Communication
 *
 * MessageBody Additions to base Class
 * 
 * For CODE_INSERT and CODE_UPDATE, Client also specified the other servers to update
 * This is inserted right after the DataBlock
 *
 * Size Bytes Included (unsginedByte)
 * Otherservers: <S1><S2>.. Each server name is an unsigned byte
 * 
 */
#ifndef CLIENTTOSERVER_H
#define CLIENTTOSERVER_H
#include "protocol.h"
#include <vector>

//Data Type
typedef unsigned char ServerName;
class ClientToServer: public Protocol
{
public:
void reset();
void addServer(ServerName serverName){serverNames.push_back(serverName);}
ClientToServer();
~ClientToServer(){}
const std::vector<ServerName>&  getServerNames(){return serverNames;}

private:
int parseMsg();
int createBody();
std::vector<ServerName> serverNames;

};
#endif

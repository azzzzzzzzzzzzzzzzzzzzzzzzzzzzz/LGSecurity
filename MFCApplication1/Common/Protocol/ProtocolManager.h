#pragma once

#include <string>
#include "BaseProtocol.h"
#include "MyProtocol.h"
using namespace std;
class CProtocolManager
{
public:
	long long getTimestamp();
	// size_t getLoginMsgToServer(unsigned char* buff, const string id, const string pw);
	// size_t getLoginMsgToClient(unsigned char* buff, const int response, const string id, const int authority);

	// size_t getImageSendStopMsgToServer(unsigned char* buff, const int mode);
	// size_t getImageMsgToClient(unsigned char* buff, const int mode, const string id, const int imgSize, const char* imgData);

	// string parseImageMsg(const unsigned char* dataToBeParsed, const int dataSize, string& id, int& mode, long long& timestamp, int& imgSize);
	
	unsigned char *make_packet(CBaseProtocol &proto, size_t *pkt_size);
	CBaseProtocol *parse_packet(MyPacket *ppkt);
	// unsigned char *make_packet(unsigned char *data);
	CBaseProtocol *create_protocol_instance(MsgReq id);

};


#pragma once

#include <string>
#include "BaseProtocol.h"
#include "MyProtocol.h"
using namespace std;
class CProtocolManager
{
public:
	long long getTimestamp();
	
	unsigned char *make_packet(CBaseProtocol &proto, size_t *pkt_size);
	CBaseProtocol *parse_packet(MyPacket *ppkt);
	CBaseProtocol *create_protocol_instance(MsgReq id);
};


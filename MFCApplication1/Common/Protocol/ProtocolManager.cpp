#include "pch.h"
#include "ProtocolManager.h"

#include "../../ProtocolDef.h"
#include <chrono>

long long 
CProtocolManager::getTimestamp() {
  const auto p1 = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             p1.time_since_epoch())
      .count();
}

CBaseProtocol *
CProtocolManager::create_protocol_instance(MsgReq id)
{
	CBaseProtocol *cpkt = nullptr;
	switch (id) 
	{
		case MSG_LOGIN: 
			cpkt = new CLoginProtocol();
			break;
		case MSG_IMAGE: 
			cpkt = new CImageProtocol();
			break;
		case MSG_CONTROL_MODE: 
			cpkt = new CControlModeProtocol();
			break;
		case MSG_SERVER_SETTING: 
			cpkt = new CServerSettingProtocol();
			break;
		case MSG_VIDEO_FILE_LIST: 
			cpkt = new CVideoFileListProtocol();
			break;
		case MSG_START_LEARNING_MODE:
			cpkt = new CLearningModeProtocol();
			break;
		case MSG_ACK: 
			cpkt = new CAckProtocol();
			break;
		default:
			break;
	}
	return cpkt;
}

unsigned char * 
CProtocolManager::make_packet(CBaseProtocol &proto, size_t *pkt_size) {
	size_t leng=proto.getSize()+sizeof(MyPacketHeader);
	MyPacket *pkt=(MyPacket *)new unsigned char[leng];
	*pkt_size=leng;
	memcpy(pkt->hdr.head,"SB1T",4);
	pkt->hdr.msgtype = proto.msg_type;	
	pkt->hdr.timestamp =getTimestamp();
	pkt->hdr.size=leng;
	proto.serializeToArray(pkt->payload);
	printf("Packetizing... packet type=%d\n", proto.msg_type);
	printf("Packetizing... packet size=%zd\n", leng);
	printf("Packetizing... packet time=%u\n", pkt->hdr.timestamp);
	return (unsigned char*)pkt;	
}


CBaseProtocol *CProtocolManager::parse_packet(MyPacket *ppkt) {
	CBaseProtocol *cpkt = nullptr;
	size_t payload_size = ppkt->hdr.size - sizeof(MyPacketHeader);
	printf("pkt header : length=%d  head=[%c%c%c%c]\n", ppkt->hdr.size,ppkt->hdr.head[0],ppkt->hdr.head[1],ppkt->hdr.head[2],ppkt->hdr.head[3]);
	if (ppkt->hdr.head[0]=='S' && ppkt->hdr.head[1]=='B' && ppkt->hdr.head[2]=='1' && ppkt->hdr.head[3]=='T' ) 
	{
		printf("pkt header : msgtype=%d\n", ppkt->hdr.msgtype);
		printf("pkt header : timestamp=%d\n", ppkt->hdr.timestamp);
		cpkt=create_protocol_instance((MsgReq)ppkt->hdr.msgtype);
		if (cpkt) cpkt->deSerialize(ppkt->payload, payload_size);
	}
	return cpkt;
}

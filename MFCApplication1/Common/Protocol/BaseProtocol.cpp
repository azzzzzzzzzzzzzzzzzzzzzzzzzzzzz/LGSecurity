#include "pch.h"
#include "BaseProtocol.h"
#include <string>
//#include <glib.h>

CBaseProtocol::CBaseProtocol()
{
	pmsg=nullptr;
	msg_type=0;
}
CBaseProtocol::CBaseProtocol(uint32_t type, ::google::protobuf::Message* pprotobuf_msg)
{
	pmsg=pprotobuf_msg;
	msg_type=type;
	printf("CBaseProtocol::CBaseProtocol() pmsg=0x%p\n",pmsg);
}

CBaseProtocol::~CBaseProtocol()
{
    
}

void
CBaseProtocol::set_msg(uint32_t type, ::google::protobuf::Message* pprotobuf_msg)
{
	pmsg=pprotobuf_msg;
	msg_type=type;
}

unsigned char* 
CBaseProtocol::serializeToArray(unsigned char* buffer)
{
	bool result = pmsg->SerializeToArray(buffer, pmsg->ByteSizeLong());
	if (true == result) {
		return buffer;
	}
	else {
		// TODO : error
		return NULL;
	}
}

size_t 
CBaseProtocol::getSize()
{
	printf("CBaseProtocol::getSize()=%zd\n",pmsg->ByteSizeLong());
	return (pmsg->ByteSizeLong());
}

//gboolean //glib
bool
CBaseProtocol::deSerialize(
	const unsigned char* serializedBuffer,
	const int serializedBufferSize)
{
	printf("CBaseProtocol::deSerialize()+\n");
	return pmsg->ParseFromArray(serializedBuffer, serializedBufferSize);
}


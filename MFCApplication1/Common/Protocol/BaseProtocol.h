#pragma once

//#include <glib.h>
#include <string>
#include <google/protobuf/message.h>
#include "ProtocolDef.h"

using namespace std;




class CBaseProtocol
{
public:
	uint32_t msg_type;
	CBaseProtocol();
	CBaseProtocol(uint32_t type, ::google::protobuf::Message* pprotobuf_msg);
    ~CBaseProtocol();
	void set_msg(uint32_t type, ::google::protobuf::Message* pprotobuf_msg);
	size_t getSize();
	virtual unsigned char* serializeToArray(unsigned char* buffer);
	virtual bool deSerialize(
		const unsigned char* serializedBuffer,
		const int serializedBufferSize);//glib
	// virtual void deSerialize(int* msgType,
	// 	string& userId,
	// 	string& passWord,
	// 	// int* authority,
	// 	const unsigned char* serializedBuffer,
	// 	const int serializedBufferSize);
protected:
	::google::protobuf::Message* pmsg;
	
};


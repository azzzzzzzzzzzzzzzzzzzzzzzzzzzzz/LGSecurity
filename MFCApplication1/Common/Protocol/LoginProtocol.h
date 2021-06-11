#pragma once

#include <string>
#include "Msg/protocolLogin.pb.h"

using namespace std;

class LoginProtocol
{
private:
	protocol_msg::LoginMsg msg;
public:
	void setMsgType(const int type);
	void setTimestamp(const long long time);
	void setUserId(const string id);
	void setUserPw(const string password);
	void setAuthority(const int authority);
	unsigned char* serializeToArray(unsigned char* buffer);
	size_t getSize();

	void deSerialize(int* msgType,
		string& userId,
		string& passWord,
		int* authority,
		const unsigned char* serializedBuffer,
		const int serializedBufferSize);
};


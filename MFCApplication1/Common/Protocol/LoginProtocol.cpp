#include "pch.h"
#include "LoginProtocol.h"

void LoginProtocol::setMsgType(const int type)
{
	msg.set_msg_type(type);
}

void LoginProtocol::setTimestamp(const long long time)
{
	msg.set_timestamp(time);
}

void LoginProtocol::setUserId(const string id)
{
	msg.set_user_id(id);
}

void LoginProtocol::setUserPw(const string password)
{
	msg.set_password(password);
}

void LoginProtocol::setAuthority(const int authority)
{
	msg.set_authority(authority);
}

unsigned char* LoginProtocol::serializeToArray(unsigned char* buffer)
{
	size_t bufferLength = msg.ByteSizeLong();
	bool result = msg.SerializeToArray(buffer, bufferLength);
	if (true == result) {
		return buffer;
	}
	else {
		// TODO : error Ã³¸®
		return NULL;
	}
}

size_t LoginProtocol::getSize()
{
	return msg.ByteSizeLong();
}

void LoginProtocol::deSerialize(int* msgType,
	string& userId,
	string& passWord,
	int* authority,
	const unsigned char* serializedBuffer,
	const int serializedBufferSize)
{
	msg.ParseFromArray(serializedBuffer, serializedBufferSize);

	*msgType = msg.msg_type();
	userId = msg.user_id();
	passWord = msg.password();
	*authority = msg.authority();
}
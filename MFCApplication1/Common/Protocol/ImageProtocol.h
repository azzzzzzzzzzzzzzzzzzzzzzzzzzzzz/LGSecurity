#pragma once

#include <string>
#include "Msg/protocolImage.pb.h"

using namespace std;

class ImageProtocol
{
private:
	protocol_msg::ImageMsg msg;
public:
	void setMsgType(const int type);
	void setTimestamp(const long long time);
	void setUserId(const string id);
	void setImage(const int imgSize, const char* imgData);
	unsigned char* serializeToArray(unsigned char* buffer);
	size_t getSize();

	std::string deSerialize(int* msgType,
		long long* timestamp,
		string& userId,
		int* imgSize,
		const unsigned char* serializedBuffer,
		const int serializedBufferSize);
};


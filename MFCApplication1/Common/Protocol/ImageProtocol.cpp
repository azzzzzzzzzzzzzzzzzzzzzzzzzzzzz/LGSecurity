#include "pch.h"
#include "ImageProtocol.h"

void ImageProtocol::setMsgType(const int type)
{
	msg.set_msg_type(type);
}

void ImageProtocol::setTimestamp(const long long time)
{
	msg.set_timestamp(time);
}

void ImageProtocol::setUserId(const string id)
{
	msg.set_user_id(id);
}

void ImageProtocol::setImage(const int imgSize, const char* imgData)
{
	msg.set_img_size(imgSize);
	std::string data_str(reinterpret_cast<char const*>(imgData), imgSize);
	msg.set_img_data(data_str);
}

unsigned char* ImageProtocol::serializeToArray(unsigned char* buffer)
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

size_t ImageProtocol::getSize()
{
	return msg.ByteSizeLong();
}

std::string ImageProtocol::deSerialize(int* msgType,
	long long* timestamp,
	string& userId,
	int* imgSize,
	const unsigned char* serializedBuffer,
	const int serializedBufferSize)
{
	msg.ParseFromArray(serializedBuffer, serializedBufferSize);

	*msgType = msg.msg_type();
	*timestamp = msg.timestamp();
	userId = msg.user_id();
	*imgSize = msg.img_size();
	return msg.img_data();
}
#include "pch.h"
#include "ProtocolManager.h"
#include "LoginProtocol.h"
#include "ImageProtocol.h"
#include "ProtocolDef.h"
#include <chrono>

long long ProtocolManager::getTimestamp()
{
	const auto p1 = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		p1.time_since_epoch()).count();
}

size_t ProtocolManager::getLoginMsgToServer(unsigned char* buff, const string id, const string pw)
{
	LoginProtocol* msg = new LoginProtocol();
	msg->setMsgType(MsgReq::MSG_LOGIN_REGISTER_USER);
	msg->setUserId(id);
	msg->setUserPw(pw);	
	msg->setTimestamp(getTimestamp());
	if (NULL == msg->serializeToArray(buff)) {
		delete msg;
		msg = 0;
		return 0;
	}
	else {
		size_t size = msg->getSize();
		delete msg;
		msg = 0;
		return size;
	}
}

size_t ProtocolManager::getLoginMsgToClient(unsigned char* buff, const int response, const string id, const int authority)
{
	LoginProtocol* msg = new LoginProtocol();
	msg->setMsgType(response);
	msg->setUserId(id);
	msg->setAuthority(authority);
	msg->setTimestamp(getTimestamp());
	if (NULL == msg->serializeToArray(buff)) {
		delete msg;
		msg = 0;
		return 0;
	}
	else {
		size_t size = msg->getSize();
		delete msg;
		msg = 0;
		return size;
	}
}

size_t ProtocolManager::getImageSendStopMsgToServer(unsigned char* buff, const int type)
{
	ImageProtocol* msg = new ImageProtocol();
	msg->setMsgType(type);
	msg->setTimestamp(getTimestamp());
	if (NULL == msg->serializeToArray(buff)) {
		delete msg;
		msg = 0;
		return 0;
	}
	else {
		size_t size = msg->getSize();
		delete msg;
		msg = 0;
		return size;
	}
}

size_t ProtocolManager::getImageMsgToClient(unsigned char* buff, const int mode, const string id, const int imgSize, const char* imgData)
{
	ImageProtocol* msg = new ImageProtocol();
	msg->setMsgType(mode);
	msg->setTimestamp(getTimestamp());
	msg->setUserId(id);
	msg->setImage(imgSize, imgData);

	if (NULL == msg->serializeToArray(buff)) {
		delete msg;
		msg = 0;
		return 0;
	}
	else {
		size_t size = msg->getSize();
		delete msg;
		msg = 0;
		return size;
	}
}

string ProtocolManager::parseImageMsg(const unsigned char* dataToBeParsed, const int dataSize, string& id, int& mode, long long& timestamp, int& imgSize)
{
	ImageProtocol* msg = new ImageProtocol();
	return msg->deSerialize(&mode, &timestamp, id, &imgSize, dataToBeParsed, dataSize);
}

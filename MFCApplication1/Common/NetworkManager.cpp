#include "pch.h"
#include "NetworkManager.h"
#include "ProtocolDef.h"

NetworkManager::NetworkManager()
:mIsSecure(true)
{
	mProtocolManager = new ProtocolManager();
}
NetworkManager::~NetworkManager()
{
	CloseTcpConnectedPort(&mTcpConnectedPort);
	delete mProtocolManager;
	delete[] mLoginBuff;
	delete[] mControlBuff;
	delete[] mImageBuff;
	delete[] mTcpBuff;
}

void NetworkManager::makeHeader(unsigned char* buff, unsigned int size)
{
	*buff = 'S';
	*(buff + 1) = 'B';
	*(buff + 2) = '1';
	*(buff + 3) = 'T';

	memcpy(buff + 4, &size, 4);
}

bool NetworkManager::openTcpConnection()
{
	if ((mTcpConnectedPort = OpenTcpConnection(DEFAULT_IP.c_str(), DEFAULT_PORT.c_str())) == NULL)
	{
		printf(" Fail OpenTcpConnection\n");
		return false;
	}
	else
	{
		return true;
	}
}

size_t NetworkManager::sendRequestLoginToServer(const string id, const string pw)
{
	size_t msgSize = mProtocolManager->getLoginMsgToServer(&mLoginBuff[HEADER_SIZE], id, pw);
	if (0 == msgSize) {
		return 0;
	}
	makeHeader(mLoginBuff, msgSize + HEADER_SIZE);

	return WriteDataTcp(mTcpConnectedPort, mLoginBuff, msgSize + HEADER_SIZE);
}

size_t NetworkManager::sendRequestImageStartToServer()
{
	size_t msgSize = mProtocolManager->getImageSendStopMsgToServer(&mControlBuff[HEADER_SIZE], MsgReq::MSG_CONTROL_START_SEND_IMAGE);
	if (0 == msgSize) {
		return 0;
	}
	makeHeader(mControlBuff, msgSize + HEADER_SIZE);

	return WriteDataTcp(mTcpConnectedPort, mControlBuff, msgSize + HEADER_SIZE);
}

size_t NetworkManager::sendRequestImageStopToServer()
{
	size_t msgSize = mProtocolManager->getImageSendStopMsgToServer(&mControlBuff[HEADER_SIZE], MsgReq::MSG_CONTROL_STOP_SEND_IMAGE);
	if (0 == msgSize) {
		return 0;
	}
	makeHeader(mControlBuff, msgSize + HEADER_SIZE);

	return WriteDataTcp(mTcpConnectedPort, mControlBuff, msgSize + HEADER_SIZE);
}

size_t NetworkManager::sendResponseLogin(const bool isSuccess, const string id, const int authority)
{
	size_t msgSize = 0;
	if (isSuccess) {
		msgSize = mProtocolManager->getLoginMsgToClient(&mLoginBuff[HEADER_SIZE], MsgRecv::MSG_OK, id, authority);
	}
	else {
		msgSize = mProtocolManager->getLoginMsgToClient(&mLoginBuff[HEADER_SIZE], MsgRecv::MSG_LOGIN_ERROR, id, 2);
	}
	if (0 == msgSize) {
		return 0;
	}
	makeHeader(mLoginBuff, msgSize + HEADER_SIZE);
	return WriteDataTcp(mTcpConnectedPort, mLoginBuff, msgSize + HEADER_SIZE);
	return size_t();
}

size_t NetworkManager::sendImageToClient(TTcpConnectedPort* tcpConnectionPort, const string id, const int mode, const int fileSize, const char* fileBuffer)
{
	size_t msgSize = mProtocolManager->getImageMsgToClient(&mImageBuff[HEADER_SIZE], mode, id, fileSize, fileBuffer);
	if (0 == msgSize) {
		return 0;
	}
	makeHeader(mImageBuff, msgSize + HEADER_SIZE);

	return WriteDataTcp(tcpConnectionPort, mImageBuff, msgSize + HEADER_SIZE);
}

string NetworkManager::parseImageMsg(const unsigned char* dataToBeParsed, const int dataSize, string& id, int& mode, long long& timestamp, int& imgSize)
{
	return mProtocolManager->parseImageMsg(dataToBeParsed, dataSize, id, mode, timestamp, imgSize);
}

bool NetworkManager::readRecvImage(cv::Mat* Image, int& msgType, long long& timestamp, string& userId, int& imgSize)
{
	size_t msgSize = readDataTcp(mIsSecure);
	string recvImg = "";
	if (msgSize > 8 && msgSize < TCP_BUFF_SIZE) {
		recvImg = parseImageMsg(&mTcpBuff[HEADER_SIZE], msgSize - HEADER_SIZE, userId, msgType, timestamp, imgSize);
	}

	//cout << imgSize << endl;

	if (!recvImg.empty() && imgSize > 0) {
		cv::imdecode(cv::Mat(imgSize, 1, CV_8UC1, (unsigned char*)recvImg.c_str()), cv::IMREAD_COLOR, Image);
		if (!(*Image).empty()) return true;
		else return false;
	}
	else {
		return false;
	}
}

size_t NetworkManager::readDataTcp(bool isSecure)
{
	return (mIsSecure)? ReadDataTcpSecure(mTcpConnectedPort, mTcpBuff, TCP_BUFF_SIZE) : ReadDataTcp(mTcpConnectedPort, mTcpBuff, TCP_BUFF_SIZE);
}


void NetworkManager::setSecureMode(const bool mode)
{
	mIsSecure = mode;
}



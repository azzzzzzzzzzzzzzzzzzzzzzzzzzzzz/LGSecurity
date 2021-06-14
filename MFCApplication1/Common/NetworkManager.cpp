#include "pch.h"
#include "NetworkManager.h"
#include "ProtocolDef.h"

NetworkManager::NetworkManager()
:mIsSecure(true)
,mIsAdmin(false)
, mMode(MODE_RUN)
{
	mProtocolManager = new CProtocolManager();
}
NetworkManager::~NetworkManager()
{
	CloseTcpConnectedPort(&mTcpConnectedPort);
	delete mProtocolManager;
	delete[] buff;
}
/*
void NetworkManager::makeHeader(unsigned char* buff, unsigned int size)
{
	*buff = 'S';
	*(buff + 1) = 'B';
	*(buff + 2) = '1';
	*(buff + 3) = 'T';

	memcpy(buff + 4, &size, 4);
}
*/

bool NetworkManager::openTcpConnection()
{
	printf("is Secure? = %d\n", mIsSecure);
	if (mIsSecure)
	{
		if ((mTcpConnectedPort = OpenTcpConnectionTLS(DEFAULT_IP.c_str(), DEFAULT_PORT_SECURE.c_str())) == NULL)
			return false;
		else
			return true;
	}
	else
	{
		if ((mTcpConnectedPort = OpenTcpConnection(DEFAULT_IP.c_str(), DEFAULT_PORT_NON_SECURE.c_str())) == NULL)
			return false;
		else
			return true;
	}
		
	
}
/*
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

size_t NetworkManager::sendRequestModeChangeToServer(UINT mode)
{
	return size_t();
}

size_t NetworkManager::sendRequestPlaySelectedVideoToServer(string videoName)
{
	return size_t();
}

size_t NetworkManager::sendRequestAddImgNameToServer(string imgName, UINT imgCnt)
{
	return size_t();
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
	return (mIsSecure)? ReadDataTcpTLS(mTcpConnectedPort, mTcpBuff, TCP_BUFF_SIZE) : ReadDataTcp(mTcpConnectedPort, mTcpBuff, TCP_BUFF_SIZE);
}*/


void NetworkManager::setSecureMode(const bool mode)
{
	mIsSecure = mode;
}

void NetworkManager::setIsAdmin(const bool admin)
{
	mIsAdmin = admin;
}

void NetworkManager::setMode(const UINT mode)
{
	mMode = mode;
}

bool NetworkManager::isSecure()
{
	return mIsSecure;
}

bool NetworkManager::isAdmin()
{
	return mIsAdmin;
}

UINT NetworkManager::getMode()
{
	return mMode;
}



bool NetworkManager::get_a_packet(Mat* pImage)
{
	ssize_t ret = 0;
	if (mIsSecure)
		ret = ReadDataTcpTLS(mTcpConnectedPort, buff, PACKET_MAX_BUFFER_SIZE);
	else
		ret = ReadDataTcp(mTcpConnectedPort, buff, PACKET_MAX_BUFFER_SIZE);
	if (ret <= PACKET_MAX_BUFFER_SIZE && ret > 0)
	{
		CProtocolManager prot_man;
		CBaseProtocol* pbase = dynamic_cast<CBaseProtocol*>(prot_man.parse_packet((MyPacket*)buff));
		if (pbase) {
			printf("Protocol type=%d\n", pbase->msg_type);
			switch (pbase->msg_type)
			{
			case MSG_IMAGE:
			{
				printf("MsgReq::MSG_IMAGE\n");
				CImageProtocol* img_pkt = dynamic_cast<CImageProtocol*>(pbase);
				cv::imdecode(cv::Mat(img_pkt->msg.img_size(), 1, CV_8UC1, (uchar*)img_pkt->msg.img_data().c_str()), cv::IMREAD_COLOR, pImage);
				// pImage->data = (uchar*)img_pkt->msg.img_data().c_str();
				//if (!(*pImage).empty()) imshow("Server", *pImage); // If a valid image is received then display it

				break;
			}
			case MSG_VIDEO_FILE_LIST:
			{
				printf("MsgReq::MSG_IMAGE\n");
				CVideoFileListProtocol* fileLIstPacket = dynamic_cast<CVideoFileListProtocol*>(pbase);
				vector<string> fileList;
				fileList.assign(fileLIstPacket->msg.filelist().begin(), fileLIstPacket->msg.filelist().end());
				break;
			}
			case MSG_LEARNING_COMPLETE:
			{
				break;
			}
			default:
				printf("Unknown msg type..\n");
				break;
			}
		}
		else {
			printf("parsing error..\n");
		}
	}
	else {
		return false;
	}
	return true;
}

bool NetworkManager::send_packet(CBaseProtocol& protocol)
{
	CProtocolManager proto_man;
	size_t leng = 0;
	unsigned char* pkt = proto_man.make_packet(protocol, &leng);

	if (pkt == NULL)
		return false;

	if (mIsSecure == true) {
		WriteDataTcpTLS(mTcpConnectedPort, pkt, leng);
	}
	else {
		WriteDataTcp(mTcpConnectedPort, pkt, leng);
	}
	delete pkt;
	return true;
}

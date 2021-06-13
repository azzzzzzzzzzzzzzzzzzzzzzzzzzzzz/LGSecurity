#pragma once

#include "opencv2/opencv.hpp" 
#include <opencv2/highgui/highgui.hpp>
#include "NetworkTCP.h"
#include "Common/Protocol/ProtocolManager.h"
#include <string>

using namespace std;

class NetworkManager
{
private:
	const int HEADER_SIZE = 8;
	const int LOGIN_BUFF_SIZE = 256;
	const int CONTROL_BUFF_SIZE = 128;
	const int IMAGE_BUFF_SIZE = 1048576;
	const int TCP_BUFF_SIZE = 1048676;
	const string DEFAULT_PORT = "55556";
	const string DEFAULT_IP = "192.168.0.116";
	TTcpConnectedPort* mTcpConnectedPort = NULL;	
	ProtocolManager* mProtocolManager;
	unsigned char* mLoginBuff = new unsigned char[LOGIN_BUFF_SIZE];
	unsigned char* mControlBuff = new unsigned char[CONTROL_BUFF_SIZE];
	unsigned char* mImageBuff = new unsigned char[IMAGE_BUFF_SIZE];
	unsigned char* mTcpBuff = new unsigned char[TCP_BUFF_SIZE];
	bool mIsSecure;
	bool mIsAdmin;
	UINT mMode;

	void makeHeader(unsigned char* buff, unsigned int size);

public:
	bool openTcpConnection();

	size_t sendRequestLoginToServer(const string id, const string pw);
	size_t sendRequestImageStartToServer();
	size_t sendRequestImageStopToServer();

	size_t sendRequestModeChangeToServer(UINT mode);
	size_t sendRequestPlaySelectedVideoToServer(string videoName);
	size_t sendRequestAddImgNameToServer(string imgName, UINT imgCnt);

	size_t sendResponseLogin(const bool isSuccess, const string id, const int authority);
	size_t sendImageToClient(TTcpConnectedPort* tcpConnectionPort, const string id, const int mode, const int fileSize, const char* fileBuffer);

	string parseImageMsg(const unsigned char* dataToBeParsed, const int dataSize, string& id, int& mode, long long& timestamp, int& imgSize);

	bool readRecvImage(cv::Mat* Image, int& msgType, long long& timestamp, string& userId, int& imgSize);
	size_t readDataTcp(bool isSecure);
	void setSecureMode(const bool mode);
	void setIsAdmin(const bool admin);
	void setMode(const UINT mode);
	bool isSecure();
	bool isAdmin();
	UINT getMode();

	NetworkManager();
	~NetworkManager();
};


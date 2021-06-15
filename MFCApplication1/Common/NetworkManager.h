#pragma once

#include "opencv2/opencv.hpp" 
#include <opencv2/highgui/highgui.hpp>
#include "NetworkTCP.h"
#include "Common/Protocol/ProtocolManager.h"
#include <string>

using namespace std;
using namespace cv;
#define PACKET_MAX_BUFFER_SIZE (1024 * 1024)

class NetworkManager
{
private:
	const int HEADER_SIZE = 8;
	const int LOGIN_BUFF_SIZE = 256;
	const int CONTROL_BUFF_SIZE = 128;
	const int IMAGE_BUFF_SIZE = 1048576;
	const int TCP_BUFF_SIZE = 1048676;
	const string DEFAULT_PORT_SECURE = "55555";
	const string DEFAULT_PORT_NON_SECURE = "50000";
	const string DEFAULT_IP = "192.168.0.116";
	TTcpConnectedPort* mTcpConnectedPort = NULL;	
	CProtocolManager* mProtocolManager;
	unsigned char* buff = new (std::nothrow) unsigned char[PACKET_MAX_BUFFER_SIZE];
	bool mIsSecure;
	bool mIsAdmin;
	UINT mMode;
	UINT mRequestType;

public:
	bool get_a_packet(Mat* pImage);
	bool send_packet(CBaseProtocol& protocol);
		
	bool openTcpConnection();
	void setSecureMode(const bool mode);
	void setIsAdmin(const bool admin);
	void setMode(const UINT mode);
	void setRequestType(const UINT reqType);
	bool isSecure();
	bool isAdmin();
	UINT getMode();
	UINT requestType();

	NetworkManager();
	~NetworkManager();
};


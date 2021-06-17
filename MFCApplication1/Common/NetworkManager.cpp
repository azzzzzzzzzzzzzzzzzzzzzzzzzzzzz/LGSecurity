#include "pch.h"
#include "NetworkManager.h"
#include "ProtocolDef.h"

NetworkManager::NetworkManager()
:mIsSecure(true)
,mIsAdmin(false)
, mMode(MODE_RUN)
, mRequestType(0)
{
	mProtocolManager = new CProtocolManager();
}
NetworkManager::~NetworkManager()
{
	CloseTcpConnectedPort(&mTcpConnectedPort);
	delete mProtocolManager;
	delete[] buff;
}

bool NetworkManager::openTcpConnection()
{
	printf("is Secure? = %d\n", mIsSecure);
	if (mIsSecure)
	{
		if ((mTcpConnectedPort = OpenTcpConnectionTLS(defaultip.c_str(), DEFAULT_PORT_SECURE.c_str())) == NULL)
			return false;
		else
			return true;
	}
	else
	{
		if ((mTcpConnectedPort = OpenTcpConnection(defaultip.c_str(), DEFAULT_PORT_NON_SECURE.c_str())) == NULL)
			return false;
		else
			return true;
	}
}

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

void NetworkManager::setRequestType(const UINT reqType)
{
	mRequestType = reqType;
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

UINT NetworkManager::requestType()
{
	return mRequestType;
}

void NetworkManager::resetStatus()
{
	mIsSecure = true;
	mIsAdmin = false;
	mMode = MODE_RUN;
	mRequestType = 0;
}

void NetworkManager::closeTCPConnection()
{
	CloseTcpConnectedPort(&mTcpConnectedPort);
}

void NetworkManager::setIPAdress(const string ip)
{
	defaultip = ip;
}

CString NetworkManager::getIPAdress()
{
	return CString();
}

bool NetworkManager::get_a_packet(Mat* pImage)
{
	ssize_t ret = 0;
	CWnd* pWnd = AfxGetApp()->GetMainWnd();
	HWND hWnd = pWnd->m_hWnd;

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
				CImageProtocol* img_pkt = dynamic_cast<CImageProtocol*>(pbase);
				cv::imdecode(cv::Mat(img_pkt->msg.img_size(), 1, CV_8UC1, (uchar*)img_pkt->msg.img_data().c_str()), cv::IMREAD_COLOR, pImage);			
			}
			break;
			case MSG_ACK:
			{		
				CAckProtocol* pkt = dynamic_cast<CAckProtocol*>(pbase);
				printf("MsgReq::MSG_OK %d\n", pkt->msg.acktype());
				if(mRequestType == MSG_LOGIN)
				{
					setIsAdmin((pkt->msg.arg() == MODE_ADMIN) ? true : false);
					if (pkt->msg.acktype() == ACK_OK)
						SendMessage(hWnd, MESSAGE_USER, MSG_LOGIN_SUCCESS, NULL);
					else
						SendMessage(hWnd, MESSAGE_USER, MSG_LOGIN_FAIL, NULL);
				}
				else if (mRequestType == MSG_CONTROL_MODE)
				{
					if (pkt->msg.acktype() == ACK_OK)
						SendMessage(hWnd, MESSAGE_USER, MSG_MODE_CHANGE_SUCCESS, NULL);
					else
						SendMessage(hWnd, MESSAGE_USER, MSG_MODE_CHANGE_FAIL, NULL);
				}
				else if (mRequestType == MSG_START_LEARNING_MODE)
				{
					printf("MSG_CONTROL_LEARNING_MODE\n");
					if (pkt->msg.acktype() == ACK_OK)
						SendMessage(hWnd, MESSAGE_USER, MSG_LEARNING_SUCCESS, NULL);
					else
						SendMessage(hWnd, MESSAGE_USER, MSG_LEARNING_FAIL, NULL);
				}
				else if (mRequestType == MSG_VIDEO_SELECT)
				{
					if (pkt->msg.acktype() == ACK_OK)
						SendMessage(hWnd, MESSAGE_USER, MSG_VIDEO_SELECTED_SUCCESS, NULL);
					else
						SendMessage(hWnd, MESSAGE_USER, MSG_VIDEO_SELECTED_FAIL, NULL);					
				}
			}
			break;
			case MSG_VIDEO_FILE_LIST:
			{
				CVideoFileListProtocol* fileLIstPacket = dynamic_cast<CVideoFileListProtocol*>(pbase);
				vector<std::string> fileList;
				fileList.assign(fileLIstPacket->msg.filelist().begin(), fileLIstPacket->msg.filelist().end());
				if(fileList.size() == 0)
					SendMessage(hWnd, MESSAGE_USER, MSG_NO_VIDEO, NULL);

				for (int i = 0; i < fileList.size(); i++)
				{
					std::string str = fileList.at(i);
					CString videoStr(str.c_str());
					SendMessage(hWnd, MESSAGE_USER, MSG_VIDEO_FILE_LIST, LPARAM(&videoStr));
				}
			}
			break;
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
	mRequestType = protocol.msg_type;
	delete pkt;
	return true;
}

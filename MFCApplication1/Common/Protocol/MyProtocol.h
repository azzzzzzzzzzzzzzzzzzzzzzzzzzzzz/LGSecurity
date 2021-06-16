#pragma once

#include <string>
#include <vector>
#include "BaseProtocol.h"
#include "Msg/protocolLogin.pb.h"

using namespace std;


/*---------------------------
		CLoginProtocol
---------------------------*/
class CLoginProtocol : public CBaseProtocol
{
public:
	CLoginProtocol();
	CLoginProtocol(const string id,const string pw);
	void setUserId(const string id);
	void setUserPw(const string password);
	protocol_msg::LoginMsg msg;
private:
};



/*---------------------------
		CImageProtocol
---------------------------*/
class CImageProtocol : public CBaseProtocol
{
public:
	CImageProtocol():CBaseProtocol(MSG_IMAGE, &msg){};
	CImageProtocol(const int imgSize, const unsigned char* imgData);
	void setImage(const int imgSize, const unsigned char* imgData);
	protocol_msg::VideoFrame msg;
};

/*---------------------------
		CControlMode
---------------------------*/
class CControlModeProtocol : public CBaseProtocol
{
public:
	CControlModeProtocol():CBaseProtocol(MSG_CONTROL_MODE, &msg){};
	CControlModeProtocol(protocol_msg::ControlMode::OpMode op);
	protocol_msg::ControlMode msg;
};

/*---------------------------
		CLearningModeProtocol
---------------------------*/
class CLearningModeProtocol : public CBaseProtocol
{
public:
	CLearningModeProtocol() :CBaseProtocol(MSG_START_LEARNING_MODE, &msg) {};
	CLearningModeProtocol(const string name, const unsigned int numImage);
	protocol_msg::LearningMode_AddUser msg;
};


/*---------------------------
		CServerSettingProtocol
---------------------------*/
class CServerSettingProtocol : public CBaseProtocol
{
public:
	CServerSettingProtocol():CBaseProtocol(MSG_SERVER_SETTING, &msg){};
	CServerSettingProtocol(protocol_msg::ServerSetting::CtlMode ctl);
	protocol_msg::ServerSetting msg;
};


/*---------------------------
		CVideoFileListProtocol
---------------------------*/
class CVideoFileListProtocol : public CBaseProtocol
{
public:
	CVideoFileListProtocol():CBaseProtocol(MSG_VIDEO_FILE_LIST, &msg){};
	CVideoFileListProtocol(vector<string>& videos);
	protocol_msg::VideoFileList msg;
};

/*---------------------------
		CVideoSelectedIndexProtocol
---------------------------*/
class CVideoSelectedIndexProtocol : public CBaseProtocol
{
public:
	CVideoSelectedIndexProtocol() :CBaseProtocol(MSG_VIDEO_SELECT, &msg) {};
	CVideoSelectedIndexProtocol(const unsigned int index);
	protocol_msg::TestMode_PlayVideo msg;
};


/*---------------------------
		CAckProtocol
---------------------------*/
class CAckProtocol : public CBaseProtocol
{
public:
	CAckProtocol():CBaseProtocol(MSG_ACK, &msg){};
	CAckProtocol(protocol_msg::Ack::AckType type, uint32_t arg);
	protocol_msg::Ack msg;
};


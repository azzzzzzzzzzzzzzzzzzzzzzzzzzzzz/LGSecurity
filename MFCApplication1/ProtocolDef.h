#pragma once
#define MESSAGE_SHOW_POPUPDLG WM_USER+10
#define MESSAGE_ADD_ITEM_TO_LIST WM_USER+11
enum SecureMode
{
	MODE_SECURE,
	MODE_NON_SECURE,
};

enum OperMode
{
	MODE_LEARNING,
	MODE_RUN,
	MODE_TEST_RUN,
};


enum  MsgReq
{
	MSG_LOGIN = 1000,
	MSG_LOGIN_REQUEST = 1001,
	MSG_IMAGE = 1002,
	MSG_CONTROL_MODE=1003,
	MSG_SERVER_SETTING = 1004,
	MSG_VIDEO_FILE_LIST = 1005,
	MSG_ACK = 9999,
} ;

enum MsgRecv
{
	MSG_OK = 1200,
	MSG_NOK = 1201,
	MSG_LEARNING_COMPLETE = 1201,
	MSG_RUN_MODE = 1202,
	MSG_LEARNING_MODE = 1203,
	MSG_TEST_MODE = 1204,
	MSG_SERVER_ERROR = 1500,
	MSG_LOGIN_DUPLICATED_ID = 1501,
	MSG_LOGIN_ERROR = 1502,
	MSG_NO_PERMISSION = 1503,
};

enum MsgUserDefine
{
	MSG_NO_VIDEO = 2000,
};


#pragma pack(push, 1)    // 1바이트 크기로 정렬
typedef  struct {
    unsigned char head[4];
    uint32_t size;
	uint32_t timestamp;
	uint32_t msgtype;
} MyPacketHeader;

typedef  struct {
	MyPacketHeader hdr;
	unsigned char payload[0];
} MyPacket;
#pragma pack(pop)

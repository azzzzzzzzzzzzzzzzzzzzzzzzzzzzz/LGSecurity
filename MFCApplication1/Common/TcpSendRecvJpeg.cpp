//------------------------------------------------------------------------------------------------
// File: TcpSendRecvJpeg.cpp
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2017 - initial version
// Send and receives OpenCV Mat Images in a Tcp Stream commpressed as Jpeg images 
//------------------------------------------------------------------------------------------------
#include "pch.h"
#include <opencv2/highgui/highgui.hpp>
#include "TcpSendRecvJpeg.h"
static  int init_values[2] = { cv::IMWRITE_JPEG_QUALITY,80 }; //default(95) 0-100
static  std::vector<int> param (&init_values[0], &init_values[0]+2);
static  std::vector<uchar> sendbuff;//buffer for coding

//-----------------------------------------------------------------
// TcpSendImageAsJpeg - Sends a Open CV Mat Image commressed as a 
// jpeg image in side a TCP Stream on the specified TCP local port
// and Destination. return bytes sent on success and -1 on failure
//-----------------------------------------------------------------
int TcpSendImageAsJpeg(TTcpConnectedPort * TcpConnectedPort,cv::Mat Image)
{
    if (!TcpConnectedPort) return -1;
    unsigned int imagesize;
    cv::imencode(".jpg", Image, sendbuff, param);
    imagesize=htonl(sendbuff.size()); // convert image size to network format
    if (WriteDataTcp(TcpConnectedPort,(unsigned char *)&imagesize,sizeof(imagesize))!=sizeof(imagesize))
    return(-1);
    return(WriteDataTcp(TcpConnectedPort,sendbuff.data(), sendbuff.size()));
}

int TcpSendImageAsJpegProtocol(TTcpConnectedPort* TcpConnectedPort, cv::Mat Image)
{
	// NetworkManager* networkManager = new NetworkManager();
    
	// cv::imencode(".jpg", Image, sendbuff, param);
	
	// networkManager->sendImageToClient(TcpConnectedPort, "lg", 1000, sendbuff.size(), (const char*)sendbuff.data());

	// delete networkManager;
	/*
	ProtoBuff* msgBuff = new ProtoBuff();
	msgBuff->setMsgType(1);
	msgBuff->setUserId("lg");
	msgBuff->setUserPw("lg_password");
	msgBuff->setAuthority(2);
	msgBuff->setOperationMode(3);
	msgBuff->setNumOfImage(0);

	unsigned int imagesize;
    cv::imencode(".jpg", Image, sendbuff, param);

	msgBuff->setImage(sendbuff.size(), (const char*)sendbuff.data());
	unsigned char* serializedData = msgBuff->serializeToArray();
	size_t serializedSize = msgBuff->getSize();

	imagesize = htonl(serializedSize);
    if (WriteDataTcp(TcpConnectedPort,(unsigned char *)&imagesize,sizeof(imagesize))!=sizeof(imagesize))
    return(-1);
    return(WriteDataTcp(TcpConnectedPort, serializedData, serializedSize));
	*/
	return 0;
}

//-----------------------------------------------------------------
// END TcpSendImageAsJpeg
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// TcpRecvImageAsJpeg - Sends a Open CV Mat Image commressed as a 
// jpeg image in side a TCP Stream on the specified TCP local port
// returns true on success and false on failure
//-----------------------------------------------------------------
bool TcpRecvImageAsJpegTLS(TTcpConnectedPort * TcpConnectedPort,cv::Mat *Image)
{
  unsigned int imagesize;
  unsigned char *buff;	/* receive buffer */   
  
  if (ReadDataTcpTLS(TcpConnectedPort,(unsigned char *)&imagesize,sizeof(imagesize))!=sizeof(imagesize)) return(false);
  
  imagesize=ntohl(imagesize); // convert image size to host format

  if (imagesize<0) return false;

  buff = new (std::nothrow) unsigned char [imagesize];
  if (buff==NULL) return false;

  if((ReadDataTcpTLS(TcpConnectedPort,buff,imagesize))==imagesize)
   {
     cv::imdecode(cv::Mat(imagesize,1,CV_8UC1,buff), cv::IMREAD_COLOR, Image );
     delete [] buff;
     if (!(*Image).empty()) return true;
     else return false;
   }
   delete [] buff;
   return false;
}

bool TcpRecvImageAsJpeg(TTcpConnectedPort * TcpConnectedPort,cv::Mat *Image)
{
  unsigned int imagesize;
  unsigned char *buff;	/* receive buffer */   
  
  if (ReadDataTcp(TcpConnectedPort,(unsigned char *)&imagesize,sizeof(imagesize))!=sizeof(imagesize)) return(false);
  
  imagesize=ntohl(imagesize); // convert image size to host format

  if (imagesize<0) return false;

  buff = new (std::nothrow) unsigned char [imagesize];
  if (buff==NULL) return false;

  if((ReadDataTcp(TcpConnectedPort,buff,imagesize))==imagesize)
   {
     cv::imdecode(cv::Mat(imagesize,1,CV_8UC1,buff), cv::IMREAD_COLOR, Image );
     delete [] buff;
     if (!(*Image).empty()) return true;
     else return false;
   }
   delete [] buff;
   return false;
}

//-----------------------------------------------------------------
// END TcpRecvImageAsJpeg
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------

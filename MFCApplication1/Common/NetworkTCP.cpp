//------------------------------------------------------------------------------------------------
// File: NetworkTCP.cpp
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2017 - initial version
// Provides the ability to send and recvive TCP byte streams for both Window and linux platforms
//------------------------------------------------------------------------------------------------
#include "pch.h"
#include <iostream>
#include <new>
#include <stdio.h>
#include <string.h>

#include "NetworkTCP.h"
//-----------------------------------------------------------------
// OpenTCPListenPort - Creates a Listen TCP port to accept
// connection requests
//-----------------------------------------------------------------
#define CERT_FILE "./certs/ca-cert.pem"
WOLFSSL_CTX* ctx = NULL;
WOLFSSL* ssl = NULL;
TTcpListenPort *OpenTcpListenPort(short localport)
{
  TTcpListenPort *TcpListenPort;
  struct sockaddr_in myaddr;

  TcpListenPort= new (std::nothrow) TTcpListenPort;  
  
  if (TcpListenPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpListenPort->ListenFd=BAD_SOCKET_FD;
  #if  defined(_WIN32) || defined(_WIN64)
  WSADATA wsaData;
  int     iResult;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) 
    {
     delete TcpListenPort;
     printf("WSAStartup failed: %d\n", iResult);
     return(NULL);
    }
#endif
  // create a socket
  if ((TcpListenPort->ListenFd= socket(AF_INET, SOCK_STREAM, 0)) == BAD_SOCKET_FD)
     {
      CloseTcpListenPort(&TcpListenPort);
      perror("socket failed");
      return(NULL);  
     }
  int option = 1; 

   if(setsockopt(TcpListenPort->ListenFd,SOL_SOCKET,SO_REUSEADDR,(char*)&option,sizeof(option)) < 0)
     {
      CloseTcpListenPort(&TcpListenPort);
      perror("setsockopt failed");
      return(NULL);
     }

  // bind it to all local addresses and pick any port number
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(localport);

  if (bind(TcpListenPort->ListenFd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
      CloseTcpListenPort(&TcpListenPort);
      perror("bind failed");
      return(NULL); 
    }
   
 
  if (listen(TcpListenPort->ListenFd,5)< 0)
  {
      CloseTcpListenPort(&TcpListenPort);
      perror("bind failed");
      return(NULL);	  
  }
  return(TcpListenPort);
}
//-----------------------------------------------------------------
// END OpenTCPListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// CloseTcpListenPort - Closes the specified TCP listen port
//-----------------------------------------------------------------
void CloseTcpListenPort(TTcpListenPort **TcpListenPort)
{
  if ((*TcpListenPort)==NULL) return;
  if ((*TcpListenPort)->ListenFd!=BAD_SOCKET_FD)  
     {
      CLOSE_SOCKET((*TcpListenPort)->ListenFd);
      (*TcpListenPort)->ListenFd=BAD_SOCKET_FD;
     }
   delete (*TcpListenPort);
  (*TcpListenPort)=NULL;
#if  defined(_WIN32) || defined(_WIN64)
   WSACleanup();
#endif
}
//-----------------------------------------------------------------
// END CloseTcpListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// AcceptTcpConnection -Accepts a TCP Connection request from a 
// Listening port
//-----------------------------------------------------------------
TTcpConnectedPort *AcceptTcpConnection(TTcpListenPort *TcpListenPort, 
                       struct sockaddr_in *cli_addr,socklen_t *clilen)
{
  TTcpConnectedPort *TcpConnectedPort;

  TcpConnectedPort= new (std::nothrow) TTcpConnectedPort;  
  
  if (TcpConnectedPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpConnectedPort->ConnectedFd= accept(TcpListenPort->ListenFd,
                      (struct sockaddr *) cli_addr,clilen);
					  
  if (TcpConnectedPort->ConnectedFd== BAD_SOCKET_FD) 
  {
	perror("ERROR on accept");
	delete TcpConnectedPort;
	return NULL;
  }
  
 int bufsize = 200 * 1024;
 if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_RCVBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
 if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_SNDBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}


 return TcpConnectedPort;
}
//-----------------------------------------------------------------
// END AcceptTcpConnection
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// OpenTCPConnection - Creates a TCP Connection to a TCP port
// accepting connection requests
//-----------------------------------------------------------------
TTcpConnectedPort *OpenTcpConnection(const char *remotehostname, const char * remoteportno)
{
    printf("Start OpenTcpConnection.\n");
  TTcpConnectedPort *TcpConnectedPort;
  struct sockaddr_in myaddr;
  int                s;
  struct addrinfo   hints;
  struct addrinfo   *result = NULL;

  TcpConnectedPort= new (std::nothrow) TTcpConnectedPort;  
  
  if (TcpConnectedPort==NULL)
     {
      fprintf(stderr, "TUdpPort memory allocation failed\n");
      return(NULL);
     }
  TcpConnectedPort->ConnectedFd=BAD_SOCKET_FD;
  #if  defined(_WIN32) || defined(_WIN64)
  WSADATA wsaData;
  int     iResult;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) 
    {
     delete TcpConnectedPort;
     printf("WSAStartup failed: %d\n", iResult);
     return(NULL);
    }
#endif
  // create a socket
   memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  
  s = getaddrinfo(remotehostname, remoteportno, &hints, &result);
  if (s != 0) 
    {
	  delete TcpConnectedPort;
      fprintf(stderr, "getaddrinfo: Failed\n");
      return(NULL);
    }
  if ( result==NULL)
    {
	  delete TcpConnectedPort;
      fprintf(stderr, "getaddrinfo: Failed\n");
      return(NULL);
    }
  if ((TcpConnectedPort->ConnectedFd= socket(AF_INET, SOCK_STREAM, 0)) == BAD_SOCKET_FD)
     {
      CloseTcpConnectedPort(&TcpConnectedPort);
	  freeaddrinfo(result);
      perror("socket failed");
      return(NULL);  
     }
  printf("1. socket \n");
  int bufsize = 200 * 1024;
  if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET,
                 SO_SNDBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
  printf("2. setsockopt SO_SNDBUF\n");
  if (setsockopt(TcpConnectedPort->ConnectedFd, SOL_SOCKET, 
                 SO_RCVBUF, (char *)&bufsize, sizeof(bufsize)) == -1)
	{
         CloseTcpConnectedPort(&TcpConnectedPort);
         perror("setsockopt SO_SNDBUF failed");
         return(NULL);
	}
  printf("3. setsockopt SO_RCVBUF\n");
  if (connect(TcpConnectedPort->ConnectedFd,result->ai_addr,result->ai_addrlen) < 0) 
          {
	    CloseTcpConnectedPort(&TcpConnectedPort);
	    freeaddrinfo(result);
            perror("connect failed");
            return(NULL);
	  }
  printf("4. connect TcpConnectedPort END\n");
  // Configure the wolfSSL library
  if (wolfSSL_Init() != WOLFSSL_SUCCESS) {
      printf("Error initializing wolfSSL library.\n");
      return(NULL);
  }
  printf("5. Configure the wolfSSL library END\n");
  // Configure wolfSSL CTX functionality
  WOLFSSL_METHOD* wolfSslMethod = wolfTLSv1_3_client_method();
  if (wolfSslMethod == NULL) {
      printf("Unable to allocate TLS v1.3 method.\n");
      return(NULL);
  }
  printf("6. Configure wolfSSL CTX functionality END\n");
  ctx = wolfSSL_CTX_new(wolfSslMethod);
  if (ctx == NULL) {
      printf("Unable get create SSL context.\n");
      return(NULL);
  }
  printf("7. wolfSSL_CTX_new END\n");
  /* Load client certificates into WOLFSSL_CTX */
  int ret = 0;
  if ((ret = wolfSSL_CTX_load_verify_locations(ctx, CERT_FILE, NULL)) != WOLFSSL_SUCCESS) {
      printf("ERROR: failed to load %s, please check the file error = %d.\n", CERT_FILE, ret);
      return(NULL);
  }
  printf("8. Load client certificates into WOLFSSL_CTX END\n");
  // Create the SSL object
  if ((ssl = wolfSSL_new(ctx)) == NULL) {
      printf("Error creating final SSL object.\n");
      return(NULL);
  }
  printf("9. Create the SSL object END\n");
  // Attach the socket file descriptor to wolfSSL
  if (wolfSSL_set_fd(ssl, TcpConnectedPort->ConnectedFd) != WOLFSSL_SUCCESS) {
      printf("Error attaching socket fd to wolfSSL.\n");
      return(NULL);
  }
  printf("10. Attach the socket file descriptor to wolfSSL END\n");
  // Call Connect for incoming connections
  if (wolfSSL_connect(ssl) != WOLFSSL_SUCCESS) {
      printf("Error establishing TLS connection to host.\n");//호스트에 대한 TLS 연결을 설정하는 동안 오류가 발생했습니다.
      return(NULL);
  }
  printf("11. Call Connect for incoming connections END\n");
  freeaddrinfo(result);	 
  return(TcpConnectedPort);
}
//-----------------------------------------------------------------
// END OpenTcpConnection
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// CloseTcpConnectedPort - Closes the specified TCP connected port
//-----------------------------------------------------------------
void CloseTcpConnectedPort(TTcpConnectedPort **TcpConnectedPort)
{
  if ((*TcpConnectedPort)==NULL) return;
  if ((*TcpConnectedPort)->ConnectedFd!=BAD_SOCKET_FD)  
     {
      CLOSE_SOCKET((*TcpConnectedPort)->ConnectedFd);
      (*TcpConnectedPort)->ConnectedFd=BAD_SOCKET_FD;
     }
   delete (*TcpConnectedPort);
  (*TcpConnectedPort)=NULL;
#if  defined(_WIN32) || defined(_WIN64)
   WSACleanup();
#endif
   printf("wolfSSL_free\n");
   if (ssl) {
       wolfSSL_free(ssl);
   }
   if (ctx) {
       wolfSSL_CTX_free(ctx);
       wolfSSL_Cleanup();
   }
}
//-----------------------------------------------------------------
// END CloseTcpListenPort
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// ReadDataTcp - Reads the specified amount TCP data 
//-----------------------------------------------------------------
ssize_t ReadDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length)
{ 
    /*
 ssize_t bytes;
 
 for (size_t i = 0; i < length; i += bytes)
    {
      if ((bytes = recv(TcpConnectedPort->ConnectedFd, (char *)(data+i), length  - i,0)) == -1) 
      {
       return (-1);
      }
    }
  return(length);*/
    ssize_t bytes = 0;
    for (size_t i = 0; i < length; i += bytes)
    {
        if ((bytes = wolfSSL_recv(ssl, (char*)(data + i), length - i,0)) < 0)
        {
            return (-1);
        }
    }
    return(length);
}
//-----------------------------------------------------------------
// END ReadDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// WriteDataTcp - Writes the specified amount TCP data 
//-----------------------------------------------------------------
ssize_t WriteDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length)
{
    /*
  ssize_t total_bytes_written = 0;
  ssize_t bytes_written;
  while (total_bytes_written != length)
    {
     bytes_written = send(TcpConnectedPort->ConnectedFd,
	                               (char *)(data+total_bytes_written),
                                  length - total_bytes_written,0);
     if (bytes_written == -1)
       {
       return(-1);
      }
     total_bytes_written += bytes_written;
   }
   return(total_bytes_written);
    */

    ssize_t total_bytes_written = 0;
    ssize_t bytes_written;
    while (total_bytes_written != length)
    {
        bytes_written = wolfSSL_write(ssl, (char*)(data + total_bytes_written), length - total_bytes_written);
        if (bytes_written < 0)
        {
            return(-1);
        }
        total_bytes_written += bytes_written;
    }
    return(total_bytes_written);
}
//-----------------------------------------------------------------
// END WriteDataTcp
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// END of File
//-----------------------------------------------------------------



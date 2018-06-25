////////////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License                                                           //
//                                                                                //
// Copyright (c) 2018, pa.eeapai@gmail.com                                        //
// All rights reserved.                                                           //
//                                                                                //
// Redistribution and use in source and binary forms, with or without             //
// modification, are permitted provided that the following conditions are met:    //
//                                                                                //
// * Redistributions of source code must retain the above copyright notice, this  //
//   list of conditions and the following disclaimer.                             //
//                                                                                //
// * Redistributions in binary form must reproduce the above copyright notice,    //
//   this list of conditions and the following disclaimer in the documentation    //
//   and/or other materials provided with the distribution.                       //
//                                                                                //
// * Neither the name of the copyright holder nor the names of its                //
//   contributors may be used to endorse or promote products derived from         //
//   this software without specific prior written permission.                     //
//                                                                                //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"    //
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE      //
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE   //
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL     //
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR     //
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER     //
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,  //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  //
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.           //
////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "WinSockWrapper.h"
#include <ws2tcpip.h>
#include <string>// Connect address parsing
#include "stdio.h"
#define _LOG(...) // { printf(__VA_ARGS__); printf("\n"); }

CWinSockWrapper::CInit::CInit()
{
  WSADATA wsaData;

  // Initialize Winsock
  int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // Can be called multiple times. Must have one WSACleanup call for each WSAStartup call.
  if ( iResult != 0 )
  {
    _LOG("WSAStartup call failed with error: %d", iResult);
  }
  m_bWsaInitialized = !iResult;
}
CWinSockWrapper::CInit::~CInit()
{
  if ( m_bWsaInitialized )
  {
    WSACleanup();
  }
}

CWinSockWrapper::CInit CWinSockWrapper::sm_init;

static bool s_setSocketBlockingMode(SOCKET socket, bool bOn)
{
  DWORD dwBlockinMode = bOn ? 0 : 1; // disable blocking
  int iResult = ioctlsocket(socket, FIONBIO, &dwBlockinMode);
  if ( iResult != NO_ERROR )
  {
    _LOG("setting blocking mode unsuccessful with error: %ld", iResult);
    return false;
  }
  return true;
}

CWinSockWrapper::CWinSockWrapper()
{
}
CWinSockWrapper::~CWinSockWrapper()
{
  Disconnect();
}
void CWinSockWrapper::StartListening(unsigned short wPort)
{
  bool bResult = true;
  bool bAddrInfoInitialized = false;
  SOCKET ListenSocket = INVALID_SOCKET;

  char achPort[32] = {'\0'};

  if ( bResult )
  {
    if ( 0 != _itoa_s(wPort, achPort, _countof(achPort), 10) )
    {
      _LOG("Invalid port %d", (int)wPort);
      bResult = false;
    }
  }

  struct addrinfo *result = NULL;
  struct addrinfo hints;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  int iResult = 0;
  if ( bResult )
  {
    iResult = getaddrinfo(NULL, achPort, &hints, &result);
    if ( iResult != 0 )
    {
      _LOG("getaddrinfo call failed with error: %d\n", iResult);
      bResult = false;
    }
    else
    {
      bAddrInfoInitialized = true;
    }
  }
  // freeaddrinfo after here

  if ( bResult )
  {
    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if ( ListenSocket == INVALID_SOCKET )
    {
      _LOG("socket call failed with WSA error: %ld\n", WSAGetLastError());
      bResult = false;
    }
    // closesocket after here
  }

  if ( bResult )
  {
    bResult = s_setSocketBlockingMode(ListenSocket, false);
  }

  if ( bResult )
  {
    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if ( iResult == SOCKET_ERROR )
    {
      _LOG("bind call failed with WSA error: %d\n", WSAGetLastError());
      bResult = false;
    }
  }
  // closesocket after here

  if ( bAddrInfoInitialized )
  {
    freeaddrinfo(result);
  }

  if ( bResult )
  {
    int nMaxConnections = 3; //SOMAXCONN;
    iResult = listen(ListenSocket, nMaxConnections);
    if ( iResult == SOCKET_ERROR )
    {
      _LOG("listen call failed with WSA error: %d\n", WSAGetLastError());
      bResult = false;
    }
  }

  if ( !bResult && (INVALID_SOCKET != ListenSocket) )
  {
    closesocket(ListenSocket);
    ListenSocket = INVALID_SOCKET;
  }

  m_listenSocket = ListenSocket;
}
void CWinSockWrapper::StopListening()
{
  if ( INVALID_SOCKET != m_listenSocket )
  {
    closesocket(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
  }
}
void CWinSockWrapper::AcceptNewConnection()
{
  if ( INVALID_SOCKET == m_listenSocket)
  {
    return;
  }
  sockaddr clientSockAddress = {0};
  int nClientSockAddressSize = sizeof(clientSockAddress);
  bool bClientJustConnected = false;
  SOCKET newScoket = INVALID_SOCKET;

  newScoket = accept(m_listenSocket, &clientSockAddress, &nClientSockAddressSize);
  if ( newScoket == INVALID_SOCKET )
  {
    int nError = WSAGetLastError();
    if ( WSAEWOULDBLOCK != nError )
    {
      _LOG("Nonblocking accept call failed with WSA error: %d\n", nError);
      StopListening();
    }
  }
  else
  {
    Disconnect();
    m_connectionSocket = newScoket;
  }
}
void CWinSockWrapper::Connect(const char *pcszDestination, unsigned short wPort)
{
  bool bResult = true;
  Disconnect();
  m_connectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if ( m_connectionSocket == INVALID_SOCKET )
  {
    _LOG("socket call failed with WSA error: %ld\n", WSAGetLastError());
    return;
  }


  struct addrinfo *result = NULL;
  DWORD dwRetval = getaddrinfo(pcszDestination, NULL, NULL, &result);
  if ( dwRetval != 0 )
  {
    _LOG("getaddrinfo call failed with error: %d\n", dwRetval);
    return;
  }

  //----------------------
  // The sockaddr_in structure specifies the address family,
  // IP address, and port of the server to be connected to.
  struct sockaddr_in clientService;
  clientService.sin_family = AF_INET;
  clientService.sin_addr.s_addr = ((sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;//inet_addr(m_strIP.c_str());
  clientService.sin_port = htons(wPort);

  //----------------------
  // Connect to server.
  int iResult = ::connect(m_connectionSocket, (SOCKADDR*)&clientService, sizeof(clientService));
  if ( iResult == SOCKET_ERROR )
  {
    _LOG("connect call failed with WSA error: %d\n", WSAGetLastError());
    closesocket(m_connectionSocket);
    m_connectionSocket = INVALID_SOCKET;
    return ;
  }
  return ;

}
void CWinSockWrapper::Disconnect()
{
  if ( INVALID_SOCKET != m_connectionSocket )
  {
    closesocket(m_connectionSocket);
    m_connectionSocket = INVALID_SOCKET;
  }
}

void CWinSockWrapper::ReceiveData(unsigned char *pbyNewData, unsigned long dwNumMaxBytes, unsigned long *pdwNumBytesReceived)
{
  *pdwNumBytesReceived = 0;
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to receive data");
    return;
  }
  if ( !s_setSocketBlockingMode(m_connectionSocket, false) )
  {
    Disconnect();
    return;
  }
  int iResult = recv(m_connectionSocket, (char *)pbyNewData, dwNumMaxBytes, 0);
  if ( iResult > 0 )
  {
    *pdwNumBytesReceived = iResult;
  }
  else if ( iResult == 0 )
  {
    _LOG("Other side disconnected");
    Disconnect(); // other side disconnected
    return;
  }
  else
  {
    iResult = WSAGetLastError();
    if ( WSAEWOULDBLOCK != iResult )
    {
      _LOG("recv call failed with WSA error: %d", iResult);
      Disconnect();
      return;
    }
    // no data received
  }
  if ( !s_setSocketBlockingMode(m_connectionSocket, true) )
  {
    Disconnect();
  }
}
void CWinSockWrapper::SendData(const unsigned char *pbyData, unsigned long dwNumBytes, unsigned long *pdwNumBytesSent)
{
  if (pdwNumBytesSent)
  {
    *pdwNumBytesSent = 0;
  }
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to send data");
    return;
  }
  
  int nResult = send(m_connectionSocket, (char*)pbyData, dwNumBytes, 0);
  if ( nResult < 0 )
  {
    _LOG("send call failed with WSA error: %d\n", WSAGetLastError());
    Disconnect();
  }
  else
  {
    if (pdwNumBytesSent)
    {
      *pdwNumBytesSent = nResult;
    }
  }
}

bool CWinSockWrapper::IsServerOK() const
{
  return INVALID_SOCKET != m_listenSocket;
}

bool CWinSockWrapper::IsClientOK() const
{
  return INVALID_SOCKET != m_connectionSocket;
}

int CWinSockWrapper::GetStatus() const
{
  if ( IsClientOK() )
  {
    return ICommDevice::connectionConnected;
  }
  if ( IsServerOK() )
  {
    return ICommDevice::connectionReady;
  }
  return ICommDevice::connectionDown;
}

int CWinSockWrapper::ListConnection(unsigned long dwConnection, char * pszDestination, int nMaxAddressLength)
{
  return 0;
}

void CWinSockWrapper::Connect(const char * pcszWhereTo)
{
  if (!pcszWhereTo)
  {
    AcceptNewConnection();
    return;
  }
  // format: [address:]port
  std::string strPort = pcszWhereTo;
  std::string strAddress;
  size_t pos = 0;
  if ( (pos = strPort.find(':')) != std::string::npos )
  {
    strAddress = strPort.substr(0, pos);
    strPort = strPort.substr(++pos);
  }
  unsigned short wPort = (unsigned short)std::stoi(strPort);

  if ( strAddress.size() )
  {
    Connect(strAddress.c_str(), wPort);
  }
  else
  {
    StartListening(wPort);
  }
}

void CWinSockWrapper::Send(const unsigned char * pbyData, unsigned long dwByteCount, unsigned long * pdwSentByteCount)
{
  SendData(pbyData, dwByteCount, pdwSentByteCount);
}

void CWinSockWrapper::Receive(unsigned char * pbyDestination, unsigned long dwMaxByteCount, unsigned long * pdwHowManyBytes)
{
  ReceiveData(pbyDestination, dwMaxByteCount, pdwHowManyBytes);
}
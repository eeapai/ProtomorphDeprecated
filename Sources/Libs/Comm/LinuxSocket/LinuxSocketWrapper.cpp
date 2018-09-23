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
#include "LinuxSocketWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <string>

#define _LOG(...) // { printf(__VA_ARGS__); printf("\n"); }
#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(a[0]))
#endif

static int getErrno()
{
  int nErrno = errno;
  return nErrno;
}

static bool s_makeSocketNonBlocking(int nSocket)
{
  int flags = 0;
  if ( (flags = fcntl(nSocket, F_GETFL)) < 0 )
  {
    _LOG("ERROR getting socket flags");
    return false;
  }

  if ( fcntl(nSocket, F_SETFL, flags | O_NONBLOCK) < 0 )
  {
    _LOG("ERROR setting non blocking mode");
    return false;
  }

  return true;
}

CLinuxSocketWrapper::CLinuxSocketWrapper()
{
}
void CLinuxSocketWrapper::StartListening(unsigned short wPort)
{
  int ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if ( ListenSocket < 0 )
  {
    _LOG("ERROR creating socket");
    return;
  }

  if ( !s_makeSocketNonBlocking(ListenSocket) )
  {
    return;
  }

  struct sockaddr_in serv_addr = { 0 };
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(wPort);
  if ( bind(ListenSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 )
  {
    _LOG("ERROR on binding address to socket");
    return;
  }
  listen(ListenSocket, 5);
  m_listenSocket = ListenSocket;
}
void CLinuxSocketWrapper::StopListening()
{
  if ( INVALID_SOCKET != m_listenSocket )
  {
    close(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
  }
}
void CLinuxSocketWrapper::AcceptNewConnection(unsigned long dwTimeout_ms /*= 100*/)
{
  if ( INVALID_SOCKET == m_listenSocket )
  {
    return;
  }
  int nTimeout_ms = (int)dwTimeout_ms;  // #TODO_Types
  struct pollfd fd2poll;
  if ( dwTimeout_ms )
  {
    fd2poll.fd = m_listenSocket;
    fd2poll.events = POLLIN | POLLPRI | POLLRDHUP;
    fd2poll.revents = 0;
    poll(&fd2poll, 1, nTimeout_ms);
  }

  sockaddr_in clientSockAddress = {0};
  socklen_t  nClientSockAddressSize = sizeof(clientSockAddress);
  int newScoket = accept(m_listenSocket, (struct sockaddr *) &clientSockAddress, &nClientSockAddressSize);
  if ( newScoket < 0 )
  {
    int nErrno = getErrno();
    if ( EAGAIN == nErrno )
    {
      return;
    }
    if ( EWOULDBLOCK == nErrno )
    {
      return;
    }
    _LOG("ERROR from nonblocking accept call with errno %d\n", nErrno);
    StopListening();
    return;
  }

  DisconnectClient();
  m_connectionSocket = newScoket;
}
void CLinuxSocketWrapper::Connect(const char *pcszDestination, unsigned short wPort)
{
  DisconnectClient();

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;        // Allow IPv4 only (AF_INET6 for IPv6; AF_UNSPEC either)
  hints.ai_socktype = SOCK_STREAM;  // SOCK_DGRAM for datagram socket
  hints.ai_flags = AI_NUMERICSERV;  // service is port number string
  hints.ai_protocol = 0;            // Any protocol

  char achPort[8] = { 0 };
  sprintf(achPort, "%d", wPort);

  struct addrinfo *result;
  int nResult = getaddrinfo(pcszDestination, achPort, &hints, &result);   // returns a list of address structures
  if ( nResult )
  {
    _LOG("ERROR from getaddrinfo call; returned %d", nResult);
    return;
  }
  // use first address
  m_connectionSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if ( m_connectionSocket < 0 )
  {
    m_connectionSocket = INVALID_SOCKET;
    freeaddrinfo(result);
    _LOG("ERROR creating socket with errno %ld\n", getErrno());
    return;
  }

  if ( 0 > connect(m_connectionSocket, result->ai_addr, result->ai_addrlen) )
  {
    close(m_connectionSocket);
    m_connectionSocket = INVALID_SOCKET;
    freeaddrinfo(result);
    _LOG("ERROR connecting with errno %ld\n", getErrno());
  }
}

void CLinuxSocketWrapper::DisconnectClient()
{
  if ( INVALID_SOCKET != m_connectionSocket )
  {
    close(m_connectionSocket);
    m_connectionSocket = INVALID_SOCKET;
  }
}

uint32_t CLinuxSocketWrapper::ReceiveData(void *pDestination, uint32_t dwNumMaxBytes, unsigned long dwTimeout_ms /*= 100*/)
{
  if ( !IsClientOK() )
  {
    return 0;
  }
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to receive data");
    return 0;
  }
  
  //int nByteCountAvailable;
  //ioctl(m_connectionSocket, FIONREAD, &nByteCountAvailable);

  int nTimeout_ms = (int)dwTimeout_ms;  // #TODO_Types

  if ( nTimeout_ms )
  {
    struct pollfd fd2poll;
    fd2poll.fd = m_connectionSocket;
    fd2poll.events = POLLIN | POLLPRI | POLLRDHUP;
    fd2poll.revents = 0;
    poll(&fd2poll, 1, nTimeout_ms);
  }

  int nNumMaxBytes = (int)dwNumMaxBytes;  // #TODO_Types
  ssize_t nResult = recv(m_connectionSocket, (char *)pDestination, nNumMaxBytes, MSG_DONTWAIT);
  if ( nResult > 0 )
  {
    return (uint32_t)nResult;
  }
  else if ( nResult == 0 )
  {
    _LOG("Other side disconnected");
    DisconnectClient(); // other side disconnected
  }
  else
  {
    int nErrno = errno;
    if ( EAGAIN == nErrno )
    {
      // no data received
      return 0;
    }
    if ( EWOULDBLOCK == nErrno )
    {
      // no data received
      return 0;
    }
    _LOG("ERROR from recv call with errno %d", nErrno);
    DisconnectClient();
  }
  return 0;
}
uint32_t CLinuxSocketWrapper::SendData(const void *pSource, uint32_t dwNumBytes)
{
  if ( !IsClientOK() )
  {
    return 0;
  }
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to send data");
    return 0;
  }

  int nNumBytes = (int)dwNumBytes;  // #TODO_Types
  ssize_t nResult = send(m_connectionSocket, (char*)pSource, nNumBytes, 0);
  if ( nResult < 0 )
  {
    _LOG("ERROR from recv call with errno %d", getErrno());
    DisconnectClient();
    return 0;
  }
  return static_cast<uint32_t>(nResult);
}

bool CLinuxSocketWrapper::IsServerOK() const
{
  return INVALID_SOCKET != m_listenSocket;
}

bool CLinuxSocketWrapper::IsClientOK() const
{
  // #TODO_SOCKET check if still connected. Not a problem if communicate periodically.
  return INVALID_SOCKET != m_connectionSocket;
}

int CLinuxSocketWrapper::GetStatus() const
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

void CLinuxSocketWrapper::Connect(const char * pcszWhereTo)
{
  if ( !pcszWhereTo )
  {
    // Accept for servers
    if ( IsServerOK() )
    {
      AcceptNewConnection();
    }
  }
  else
  {
    // format: [address:]port
    std::string strPort = pcszWhereTo;
    std::string strAddress;
    size_t pos = 0;
    if ( (pos = strPort.find(':')) != std::string::npos )
    {
      strAddress = strPort.substr(0, pos);
      strPort = strPort.substr(++pos);
    }
    
    bool bOnlyDigitsInPortStr = strspn(strPort.c_str(), "0123456789") == strPort.size();
    if ( !bOnlyDigitsInPortStr )
    {
      return;
    }
    uint16_t wPort = (unsigned short)std::stoi(strPort);

    if ( strAddress.size() )
    {
      Connect(strAddress.c_str(), wPort);
    }
    else
    {
      StartListening(wPort);
    }
  }
}

uint32_t CLinuxSocketWrapper::Send(const void * pSource, uint32_t dwByteCount)
{
  return SendData(pSource, dwByteCount);
}

uint32_t CLinuxSocketWrapper::Receive(void *pDestination, uint32_t dwMaxByteCount)
{
  return ReceiveData(pDestination, dwMaxByteCount);
}

void CLinuxSocketWrapper::Disconnect()
{
  if ( INVALID_SOCKET != m_connectionSocket )
  {
    DisconnectClient();
  }
  else
  {
    StopListening();
  }
}


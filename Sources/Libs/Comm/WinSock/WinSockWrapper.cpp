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
#include "stdio.h"
#define _LOG(...) // { printf(__VA_ARGS__); printf("\n"); }

#pragma comment(lib, "ws2_32.lib")

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

static uint16_t s_portAndHostFromDestination(const char *pcszDestination, std::string &rstrHost)
{
  // format: [address:]port
  std::string strPort = pcszDestination;
  size_t pos = 0;
  if ( (pos = strPort.find(':')) != std::string::npos )
  {
    rstrHost = strPort.substr(0, pos);
    strPort = strPort.substr(++pos);
  }
  return (unsigned short)std::stoi(strPort);
}

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

static bool s_setSocketSharedMode(SOCKET socket, bool bOn)
{
  uint32_t dwShared = bOn ? 1 : 0;
  int nResult = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&dwShared, sizeof(dwShared));
  if ( nResult < 0 )
  {
    _LOG("Setting shared socket mode unsuccessful with WSA error: %d", WSAGetLastError());
    return false;
  }
  return true;
}

static bool s_addSocketMembership(SOCKET socket, const char *pcszGroupIP, const char *pcszLocalBindIP = nullptr)
{
  struct ip_mreq mreq;
  inet_pton(AF_INET, pcszGroupIP, &mreq.imr_multiaddr);
  if ( pcszLocalBindIP )
  {
    inet_pton(AF_INET, pcszLocalBindIP, &mreq.imr_interface);
  }
  else
  {
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  }
  int nResult = setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
  if ( nResult < 0 )
  {
    _LOG("Adding socket membership unsuccessful with WSA error: %d", WSAGetLastError());
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
  if (IsServerOK())
  {
    StopListening();
  }
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
void CWinSockWrapper::AcceptNewConnection() // #TODO_BLOCKING_ACCEPT maybe not with select but with listen thread
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
    else
    {
      Sleep(10); // #TODO_BLOCKING_ACCEPT remove
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

uint32_t CWinSockWrapper::Receive(void *pDestination, uint32_t dwNumMaxBytes)
{
  uint32_t dwNumBytesReceived = 0;
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to receive data");
    return 0;
  }
  if ( !s_setSocketBlockingMode(m_connectionSocket, false) )
  {
    Disconnect();
    return 0;
  }
  int iResult = recv(m_connectionSocket, (char *)pDestination, dwNumMaxBytes, 0);
  if ( iResult > 0 )
  {
    dwNumBytesReceived = iResult;
  }
  else if ( iResult == 0 )
  {
    _LOG("Other side disconnected");
    Disconnect(); // other side disconnected
    return 0;
  }
  else
  {
    iResult = WSAGetLastError();
    if ( WSAEWOULDBLOCK != iResult )
    {
      _LOG("recv call failed with WSA error: %d", iResult);
      Disconnect();
      return 0;
    }
    // no data received
  }
  if ( !s_setSocketBlockingMode(m_connectionSocket, true) )
  {
    Disconnect();
    return 0;
  }
  return dwNumBytesReceived;
}
uint32_t CWinSockWrapper::Send(const void *pSource, uint32_t dwNumBytes)
{
  if ( INVALID_SOCKET == m_connectionSocket )
  {
    _LOG("No connection to send data");
    return 0;
  }
  
  int nResult = send(m_connectionSocket, (char*)pSource, dwNumBytes, 0);
  if ( nResult < 0 )
  {
    _LOG("send call failed with WSA error: %d\n", WSAGetLastError());
    Disconnect();
    return 0;
  }

  return nResult;
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
  std::string strAddress;
  unsigned short wPort = s_portAndHostFromDestination(pcszWhereTo, strAddress);

  if ( strAddress.size() )
  {
    Connect(strAddress.c_str(), wPort);
  }
  else
  {
    StartListening(wPort);
  }
}



CWinSockMulticastWrapper::CWinSockMulticastWrapper()
{
}

CWinSockMulticastWrapper::~CWinSockMulticastWrapper()
{
}

void CWinSockMulticastWrapper::SetLocalIPv4Bind(const char * pcszIP)
{
  m_strBindIP = pcszIP;
}

void CWinSockMulticastWrapper::GetIPv4Source(uint8_t * pbyIP) const
{
  pbyIP[0] = m_receiveAddr.sin_addr.S_un.S_un_b.s_b1;
  pbyIP[1] = m_receiveAddr.sin_addr.S_un.S_un_b.s_b2;
  pbyIP[2] = m_receiveAddr.sin_addr.S_un.S_un_b.s_b3;
  pbyIP[3] = m_receiveAddr.sin_addr.S_un.S_un_b.s_b4;
}

void CWinSockMulticastWrapper::Connect(const char * pcszWhereTo /*= ""*/)
{
  m_wPort = s_portAndHostFromDestination(pcszWhereTo, m_strGroup);
  prepareForReception();

  bool bResult = true;
  m_transmitSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if ( m_transmitSocket == INVALID_SOCKET )
  {
    _LOG("socket call failed with WSA error: %ld", WSAGetLastError());
    bResult = false;
  }

  if ( bResult )
  {
    bResult = s_setSocketSharedMode(m_transmitSocket, true);
    if ( !bResult )
    {
      closesocket(m_transmitSocket);
      m_transmitSocket = INVALID_SOCKET;
    }
  }

  if ( bResult )
  {
    struct sockaddr_in bindAddr;

    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family = AF_INET;
    if ( m_strBindIP.length() )
    {
      inet_pton(AF_INET, m_strBindIP.c_str(), &bindAddr.sin_addr);
    }
    else
    {
      bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    bindAddr.sin_port = htons(m_wPort);

    int nResult = 0;
    if ( SOCKET_ERROR == bind(m_transmitSocket, (struct sockaddr*) &bindAddr, sizeof(bindAddr)) )
    {
      int nResult = WSAGetLastError();
      _LOG("Transmit socket bind to address failed with WSA error: %ld", nResult);
    }
  }
}

int CWinSockMulticastWrapper::GetStatus() const
{
  if ( isClientOK() )
  {
    return ICommDevice::connectionConnected;
  }
  if ( isReceptionOK() )
  {
    return ICommDevice::connectionReady;
  }
  return ICommDevice::connectionDown;
}

uint32_t CWinSockMulticastWrapper::Send(const void * pSource, uint32_t dwByteCount)
{
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, m_strGroup.c_str(), &addr.sin_addr);
  addr.sin_port = htons(m_wPort);

  int nResult = sendto(m_transmitSocket, (const char *)pSource, dwByteCount, 0, (struct sockaddr*)(&addr), sizeof(addr));
  if ( nResult == SOCKET_ERROR )
  {
    nResult = WSAGetLastError();
    if ( WSAEWOULDBLOCK != nResult )
    {
      _LOG("sendto call failed with WSA error: %d", nResult);
      closesocket(m_transmitSocket);
      m_transmitSocket = INVALID_SOCKET;
    }
    return 0;
  }
  return nResult;
}

uint32_t CWinSockMulticastWrapper::Receive(void * pDestination, uint32_t dwMaxByteCount)
{
  if ( INVALID_SOCKET == m_listenSocket )
  {
    _LOG("No connection to receive data");
    return 0;
  }

  int nAddrsize = sizeof(m_receiveAddr);
  int nReceivedCount = recvfrom(m_listenSocket, (char *)pDestination, dwMaxByteCount, 0, (struct sockaddr *) &m_receiveAddr, &nAddrsize);
  if ( nReceivedCount == SOCKET_ERROR )
  {
    int nResult = WSAGetLastError();
    if ( WSAEWOULDBLOCK != nResult )
    {
      _LOG("recvfrom call failed with WSA error: %d", nResult);
      closesocket(m_listenSocket);
      m_listenSocket = INVALID_SOCKET;
    }
    return 0;
  }
  return nReceivedCount;
}

void CWinSockMulticastWrapper::Disconnect()
{
  if ( INVALID_SOCKET != m_listenSocket )
  {
    closesocket(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
  }
  if ( INVALID_SOCKET != m_transmitSocket )
  {
    closesocket(m_transmitSocket);
    m_transmitSocket = INVALID_SOCKET;
  }
}

void CWinSockMulticastWrapper::prepareForReception()
{
  bool bResult = true;
  SOCKET listenSocket = INVALID_SOCKET;
  if ( bResult )
  {
    listenSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if ( listenSocket == INVALID_SOCKET )
    {
      _LOG("socket call failed with WSA error: %ld", WSAGetLastError());
      bResult = false;
    }
  }

  if ( bResult )
  {
    bResult = s_setSocketSharedMode(listenSocket, true);
  }

  struct sockaddr_in bindAddr;

  memset(&bindAddr, 0, sizeof(bindAddr));
  bindAddr.sin_family = AF_INET;
  if ( m_strBindIP.length() )
  {
    inet_pton(AF_INET, m_strBindIP.c_str(), &bindAddr.sin_addr);
  }
  else
  {
    bindAddr.sin_addr.s_addr =  htonl(INADDR_ANY);
  }
  bindAddr.sin_port = htons(m_wPort);

  if ( bResult )
  {
    if ( SOCKET_ERROR == bind(listenSocket, (struct sockaddr*)&bindAddr, sizeof(bindAddr)) )
    {
      bResult = false;
      int nResult = WSAGetLastError();
      _LOG("Listen socket bind to address failed with WSA error: %ld", nResult);
    }
  }

  if ( (INVALID_SOCKET != listenSocket) && !bResult )
  {
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
  }

  if ( bResult )
  {
    bResult = s_addSocketMembership(listenSocket, m_strGroup.c_str(), m_strBindIP.length() ? m_strBindIP.c_str() : nullptr);
  }

  m_listenSocket = listenSocket;
}

bool CWinSockMulticastWrapper::isReceptionOK() const
{
  return INVALID_SOCKET != m_listenSocket;
}

bool CWinSockMulticastWrapper::isClientOK() const
{
  return INVALID_SOCKET != m_transmitSocket;
}


////////////////////////////////////////////////////////////////////

#include <iphlpapi.h>
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")



bool CWinNetHelpers::SHostAdapter::IsEthernetNetworkInterface() const
{
  return IF_TYPE_ETHERNET_CSMACD == m_dwIfType;
}
bool CWinNetHelpers::SHostAdapter::IsWirelessNetworkInterface() const
{
  return IF_TYPE_IEEE80211 == m_dwIfType;
}
bool CWinNetHelpers::SHostAdapter::IsUp() const
{
  return IfOperStatusUp == m_dwOperStatus;
}

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define IPLIST_PRITF(...)
//#define IPLIST_PRITF printf

void CWinNetHelpers::HostAdaptersList(uint32_t dwFlags, std::vector<SHostAdapter> &rvHostAdapters)
{
  if ( dwFlags )
  {
    return;
  }
  /* Declare and initialize variables */

  DWORD dwSize = 0;
  DWORD dwRetVal = 0;

  unsigned int i = 0;

  // Set the flags to pass to GetAdaptersAddresses
  ULONG flags =
    GAA_FLAG_SKIP_ANYCAST |
    GAA_FLAG_SKIP_MULTICAST |
    GAA_FLAG_SKIP_DNS_SERVER |
    GAA_FLAG_INCLUDE_PREFIX |
    GAA_FLAG_SKIP_FRIENDLY_NAME |
    GAA_FLAG_INCLUDE_ALL_INTERFACES;

  // default to unspecified address family (both)
  ULONG family = AF_UNSPEC;

  LPVOID lpMsgBuf = NULL;

  PIP_ADAPTER_ADDRESSES pAddresses = NULL;
  ULONG outBufLen = 0;
  ULONG Iterations = 0;

  PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
  PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
  PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
  PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
  IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
  IP_ADAPTER_PREFIX *pPrefix = NULL;

  family = AF_INET;

  outBufLen = WORKING_BUFFER_SIZE;

  do
  {
    pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
    if ( pAddresses == NULL )
    {
      return;
    }

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

    if ( dwRetVal == ERROR_BUFFER_OVERFLOW )
    {
      FREE(pAddresses);
      pAddresses = NULL;
    }
    else
    {
      break;
    }

    Iterations++;

  } while ( (dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations<MAX_TRIES) );

  if ( dwRetVal == NO_ERROR )
  {
    // If successful, output some information from the data we received
    pCurrAddresses = pAddresses;
    while ( pCurrAddresses )
    {
      SHostAdapter ha;
      IPLIST_PRITF("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n", pCurrAddresses->Length);
      IPLIST_PRITF("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
      ha.m_dwIfIndex = pCurrAddresses->IfIndex;
      IPLIST_PRITF("\tAdapter name: %s\n", pCurrAddresses->AdapterName);
      ha.m_strPermanentAdapterName = pCurrAddresses->AdapterName;

      pUnicast = pCurrAddresses->FirstUnicastAddress;
      if ( pUnicast != NULL )
      {
        for ( i = 0; pUnicast != NULL; i++ )
        {
          if ( AF_INET == pUnicast->Address.lpSockaddr->sa_family )
          {
            sockaddr_in *psai = (sockaddr_in *)(pUnicast->Address.lpSockaddr);
            char achIP[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &(psai->sin_addr), achIP, sizeof(achIP));
            ha.m_vstrUnicastIPv4Addresses.push_back(achIP);
            IPLIST_PRITF("\tUnicast IP: %s\n", achIP);
          }
          else if ( AF_INET6 == pUnicast->Address.lpSockaddr->sa_family )
          {
            sockaddr_in6 *psai = (sockaddr_in6 *)(pUnicast->Address.lpSockaddr);
            char achIP[INET6_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET6, &(psai->sin6_addr), achIP, sizeof(achIP));
            ha.m_vstrUnicastIPv6Addresses.push_back(achIP);
            IPLIST_PRITF("\tUnicast IP: %s\n", achIP);
          }
          pUnicast = pUnicast->Next;
        }
        IPLIST_PRITF("\tNumber of Unicast Addresses: %d\n", i); // More than 1? How can that be...
      }
      else
      {
        IPLIST_PRITF("\tNo Unicast Addresses\n");
      }

      pAnycast = pCurrAddresses->FirstAnycastAddress;
      if ( pAnycast )
      {
        for ( i = 0; pAnycast != NULL; i++ )
        {
          pAnycast = pAnycast->Next;
        }
        IPLIST_PRITF("\tNumber of Anycast Addresses: %d\n", i);
      }
      else
      {
        IPLIST_PRITF("\tNo Anycast Addresses\n");
      }

      pMulticast = pCurrAddresses->FirstMulticastAddress;
      if ( pMulticast )
      {
        for ( i = 0; pMulticast != NULL; i++ )
        {
          pMulticast = pMulticast->Next;
        }
        IPLIST_PRITF("\tNumber of Multicast Addresses: %d\n", i);
      }
      else
      {
        IPLIST_PRITF("\tNo Multicast Addresses\n");
      }

      pDnServer = pCurrAddresses->FirstDnsServerAddress;
      if ( pDnServer )
      {
        for ( i = 0; pDnServer != NULL; i++ )
        {
          pDnServer = pDnServer->Next;
        }
        IPLIST_PRITF("\tNumber of DNS Server Addresses: %d\n", i);
      }
      else
      {
        IPLIST_PRITF("\tNo DNS Server Addresses\n");
      }

      IPLIST_PRITF("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
      IPLIST_PRITF("\tDescription: %wS\n", pCurrAddresses->Description);
      IPLIST_PRITF("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);
      ha.m_strFriendlyAdapterName = pCurrAddresses->FriendlyName;

      if ( pCurrAddresses->PhysicalAddressLength != 0 )
      {
        IPLIST_PRITF("\tPhysical address: ");
        for ( i = 0; i<(int)pCurrAddresses->PhysicalAddressLength; i++ )
        {
          if ( i == (pCurrAddresses->PhysicalAddressLength - 1) )
          {
            IPLIST_PRITF("%.2X\n", (int)pCurrAddresses->PhysicalAddress[i]);
          }
          else
          {
            IPLIST_PRITF("%.2X-", (int)pCurrAddresses->PhysicalAddress[i]);
          }
        }
      }
      IPLIST_PRITF("\tFlags: %ld\n", pCurrAddresses->Flags);
      ha.m_dwFlags = pCurrAddresses->Flags;
      IPLIST_PRITF("\tMtu: %lu\n", pCurrAddresses->Mtu);
      IPLIST_PRITF("\tIfType: %ld\n", pCurrAddresses->IfType);
      ha.m_dwIfType = pCurrAddresses->IfType;
      IPLIST_PRITF("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
      ha.m_dwOperStatus = pCurrAddresses->OperStatus;
      IPLIST_PRITF("\tIpv6IfIndex (IPv6 interface): %u\n", pCurrAddresses->Ipv6IfIndex);
      IPLIST_PRITF("\tZoneIndices (hex): ");
      for ( i = 0; i < 16; i++ )
      {
        IPLIST_PRITF("%lx ", pCurrAddresses->ZoneIndices[i]);
      }
      IPLIST_PRITF("\n");

      IPLIST_PRITF("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
      IPLIST_PRITF("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

      pPrefix = pCurrAddresses->FirstPrefix;
      if ( pPrefix )
      {
        for ( i = 0; pPrefix != NULL; i++ )
        {
          if ( AF_INET == pPrefix->Address.lpSockaddr->sa_family )
          {
            sockaddr_in *psai = (sockaddr_in *)(pPrefix->Address.lpSockaddr);
            char achIP[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &(psai->sin_addr), achIP, sizeof(achIP));
            IPLIST_PRITF("\tPrefix IP: %s\n", achIP);
          }
          else if ( AF_INET6 == pPrefix->Address.lpSockaddr->sa_family )
          {
            sockaddr_in6 *psai = (sockaddr_in6 *)(pPrefix->Address.lpSockaddr);
            char achIP[INET6_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET6, &(psai->sin6_addr), achIP, sizeof(achIP));
            IPLIST_PRITF("\tPrefix IP: %s\n", achIP);
          }

          pPrefix = pPrefix->Next;
        }
        IPLIST_PRITF("\tNumber of IP Adapter Prefix entries: %d\n", i);
      }
      else
      {
        IPLIST_PRITF("\tNumber of IP Adapter Prefix entries: 0\n");
      }

      IPLIST_PRITF("\n");

      pCurrAddresses = pCurrAddresses->Next;
      rvHostAdapters.emplace_back(ha);
    }
  }
  else
  {
    IPLIST_PRITF("Call to GetAdaptersAddresses failed with error: %d\n", dwRetVal);
    if ( dwRetVal == ERROR_NO_DATA )
    {
      IPLIST_PRITF("\tNo addresses were found for the requested parameters\n");
    }
    else
    {
      if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        // Default language
        (LPTSTR)& lpMsgBuf, 0, NULL) )
      {
        IPLIST_PRITF("\tError: %s", lpMsgBuf);
        LocalFree(lpMsgBuf);
      }
    }
  }

  if ( pAddresses )
  {
    FREE(pAddresses);
  }
}
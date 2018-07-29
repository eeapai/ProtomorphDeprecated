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
#ifndef __WINSOCK_WRAPPER_H__
#define __WINSOCK_WRAPPER_H__

#include <winsock2.h>
#include "ICommDevice.h"
#include <string>
#include <vector>

class CWinSockWrapper : public ICommDevice
{
public:
  CWinSockWrapper();
  ~CWinSockWrapper();
private:
  class CInit
  {
  public:
    CInit();
    ~CInit();
  private:
    bool m_bWsaInitialized = false;
  };
  static CInit sm_init;

public:
  void StartListening(unsigned short wPort);
  void StopListening();
  void AcceptNewConnection();
  void Connect(const char *pcszDestination, unsigned short wPort);
  void Disconnect();

  bool IsServerOK() const;
  bool IsClientOK() const;

public:
  static int ListConnection(unsigned long dwConnection, char * pszDestination, int nMaxAddressLength);
  void Connect(const char * pcszWhereTo) override;
  int GetStatus() const override;
  uint32_t Send(const void *pSource, uint32_t dwByteCount) override;
  uint32_t Receive(void *pDestination, uint32_t dwMaxByteCount) override;

private:
  SOCKET m_listenSocket = INVALID_SOCKET;
  SOCKET m_connectionSocket = INVALID_SOCKET;
};


class CWinSockMulticastWrapper : public ICommDevice
{
public:
  CWinSockMulticastWrapper();
  ~CWinSockMulticastWrapper();

  void SetLocalIPv4Bind(const char *pcszIP);
  void GetIPv4Source(uint8_t * pbyIP) const; // Gets meaningful data after Receive returns positive value

public:
  // Inherited via ICommDevice
  void Connect(const char * pcszWhereTo = "") override;
  int GetStatus() const override;
  uint32_t Send(const void * pSource, uint32_t dwByteCount) override;
  uint32_t Receive(void * pDestination, uint32_t dwMaxByteCount) override;
  void Disconnect() override;

private:
  void prepareForReception();
  bool isReceptionOK() const;
  bool isClientOK() const;

private:
  std::string m_strBindIP;  // Set in optional SetLocalIPv4Bindcall. INADDR_ANY will be used if SetLocalBind not called.

  std::string m_strGroup;   // Multicast group IP to receive from and send to.
  uint16_t m_wPort;         // Multicat port;

  SOCKET m_listenSocket = INVALID_SOCKET;
  struct sockaddr_in m_receiveAddr; // Valid after Receive returns positive value

  SOCKET m_transmitSocket = INVALID_SOCKET;

};

class CWinNetHelpers
{
public:
  struct SHostAdapter
  {
    uint32_t m_dwIfIndex = 0;
    std::string m_strPermanentAdapterName;
    std::wstring m_strFriendlyAdapterName;
    std::vector<std::string> m_vstrUnicastIPv4Addresses;
    std::vector<std::string> m_vstrUnicastIPv6Addresses;
    std::string m_strPhysicalAddress;
    union {
      uint32_t m_dwFlags = 0;
      struct {
        ULONG DdnsEnabled : 1;
        ULONG RegisterAdapterSuffix : 1;
        ULONG Dhcpv4Enabled : 1;
        ULONG ReceiveOnly : 1;
        ULONG NoMulticast : 1;
        ULONG Ipv6OtherStatefulConfig : 1;
        ULONG NetbiosOverTcpipEnabled : 1;
        ULONG Ipv4Enabled : 1;
        ULONG Ipv6Enabled : 1;
        ULONG Ipv6ManagedAddressConfigurationSupported : 1;
      };
    };
    uint32_t m_dwIfType = 0;
    uint32_t m_dwOperStatus = 0;

    bool IsEthernetNetworkInterface() const;
    bool IsWirelessNetworkInterface() const;
    bool IsUp() const;
  };

  static void HostAdaptersList(uint32_t dwFlags, std::vector<SHostAdapter> &rvHostAdapters);
};
#endif
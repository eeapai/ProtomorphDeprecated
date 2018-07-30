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
#define _WINSOCKAPI_  // Prevents windows.h to include winsock.h
#include <Windows.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>     // std::cout
#include <string>

#include <thread>
#include <mutex>
#include <chrono>

#include "WinSockWrapper.h"
#include "DNSPacket.h"

static void s_printfHEX(const char *pData, uint32_t dwNumBytes, uint32_t dwPerLine = 8)
{
  std::string strLine;
  for ( uint32_t dwWriten = 0; dwWriten < dwNumBytes; ++dwWriten )
  {
    if ( dwWriten && !(dwWriten % dwPerLine) )
    {
      std::cout << "   ";
      std::cout << strLine;
      strLine.clear();
      std::cout << std::endl;
    }
    printf("%02X ", (unsigned char)(pData[dwWriten]));
    if ( (pData[dwWriten] >= ' ') && (pData[dwWriten] <= '~') )
    {
      strLine.append({ pData[dwWriten] });
    }
    else
    {
      strLine.append(".");
    }
  }
  size_t nPlacesTaken = strLine.length();
  size_t nPlacesToFill = dwPerLine - nPlacesTaken;
  while ( nPlacesToFill-- )
  {
    std::cout << "   ";
  }
  std::cout << "   ";
  std::cout << strLine;
  strLine.clear();
  std::cout << std::endl;
}

static const char * s_pcszUsage =
"mDNSLookup name [-h<local ip>] [-g<group ip>] [-p<port>]   \n\r"
"                                                           \n\r"
"  name     - Name to searh for                             \n\r"
"             e.g. raspberypi.local                         \n\r"
"  local ip - Single network to use in case there are       \n\r"
"             multiple connected network adapters. Give IP  \n\r"
"             of your local network adapter - find it with  \n\r"
"             'ipconfig' batch command.                     \n\r"
"             e.g. 192.168.1.2                              \n\r"
"             All live adapters will be tested by defualt.  \n\r"
"  group ip - MulticastDNS group.                           \n\r"
"             224.0.0.251 by default.                       \n\r"
"  port     - MulticastDNS port.                            \n\r"
"             5353 by default.                              \n\r"
"                                                           \n\r";

int main(int argc, const char **argv)
{
  const char * pcszGroupIP = "224.0.0.251";
  uint16_t wPort = 5353;
  const char *pcszLocalIP = nullptr;
  const char *pcszName = nullptr;

  if ( argc < 2 )
  {
    printf(s_pcszUsage);
    return -1;
  }

  for ( int nArg = 1; nArg < argc; ++nArg )
  {
    const char *pcszArg = argv[nArg];
    if ( '-' == pcszArg[0] )
    {
      switch ( pcszArg[1] )
      {
      case 'h': pcszLocalIP = &pcszArg[2]; break;
      case 'g': pcszGroupIP = &pcszArg[2]; break;
      case 'p': wPort = atoi(&pcszArg[2]); break;
      default:
        printf(s_pcszUsage);
        return -1;
        break;
      }
    }
    else if ( !pcszName )
    {
      pcszName = pcszArg;
    }
    else
    {
      printf(s_pcszUsage);
      return -1;
    }
  }

  if ( !pcszName )
  {
      printf(s_pcszUsage);
      return -1;
  }

  printf("Searching for %s on %s host\n\r", pcszName, pcszLocalIP ? pcszLocalIP : "all or only");
  printf("Using port %d on %s multicast group\n\r\n\r", wPort, pcszGroupIP);

  uint8_t abyQuery[1024];
  size_t sPacketLength = DNSMakeQueryPacket(pcszName, abyQuery, sizeof(abyQuery));
  if ( sPacketLength > sizeof(abyQuery) )
  {
    printf("Not enoug internal buffer for mDNS query creation.");
    return -1;
  }

  std::string strGroup = pcszGroupIP;
  strGroup.append(":");
  strGroup.append(std::to_string(wPort));

  std::vector<CWinSockMulticastWrapper> vMCWs;

  if ( pcszLocalIP )
  {
    CWinSockMulticastWrapper mcw;
    vMCWs.push_back(mcw);
  }
  else
  {
    std::vector<CWinNetHelpers::SHostAdapter> vHostAdapters;
    CWinNetHelpers::HostAdaptersList(0, vHostAdapters);
    for ( const CWinNetHelpers::SHostAdapter &rHA : vHostAdapters )
    {
      if ( (rHA.IsEthernetNetworkInterface() || rHA.IsWirelessNetworkInterface()) &&
            rHA.IsUp() && 
            rHA.m_vstrUnicastIPv4Addresses.size() )
      {
        //printf("%s\n", rHA.m_vstrUnicastIPv4Addresses[0].c_str());
        CWinSockMulticastWrapper mcw;
        mcw.SetLocalIPv4Bind(rHA.m_vstrUnicastIPv4Addresses[0].c_str());
        vMCWs.push_back(mcw);
      }
    }
  }
  
  std::vector<std::thread> vListeners;
  std::mutex mutexOut;
  for ( CWinSockMulticastWrapper &rmcw : vMCWs )
  {
    rmcw.Connect(strGroup.c_str());
    std::thread tListener([&pcszName, &mutexOut](CWinSockMulticastWrapper *pmcw)
    {
      char achReceived[1024 + 1];
      while ( ICommDevice::connectionDown != pmcw->GetStatus() )
      {
        uint32_t dwReceivedCount = pmcw->Receive(achReceived, sizeof(achReceived) - 1);
        if ( !dwReceivedCount )
        {
          continue;
        }
        achReceived[dwReceivedCount] = 0;
        //uint8_t abyIP[4] = { 0 };
        //pmcw->GetIPv4Source(abyIP);
        //bool bWatchedIP = false;
        //bWatchedIP = bWatchedIP || (121 == abyIP[3]);
        //bWatchedIP = bWatchedIP || (14 == abyIP[3]);
        //bWatchedIP = bWatchedIP || (62 == abyIP[3]);
        //bWatchedIP = bWatchedIP || (172 == abyIP[3]);
        //if ( !bWatchedIP )
        //{
        //  //continue;
        //}
        //std::cout << "--- New packet --- from: ";
        //printf("%d.%d.%d.%d", abyIP[0], abyIP[1], abyIP[2], abyIP[3]);
        //std::cout << std::endl;
        std::lock_guard<std::mutex> guard(mutexOut);
        DNSDumpPacket((uint8_t *)achReceived, dwReceivedCount, pcszName);
        //s_printfHEX(achReceived, dwReceivedCount, 16);
        //std::cout << std::endl;
        //std::cout << "------------------" << std::endl;
      }
    }, &rmcw);

    vListeners.push_back(std::move(tListener));
    rmcw.Send(abyQuery, sPacketLength);
  }
  
  getchar();

  //std::this_thread::sleep_for(std::chrono::seconds(3));

  for ( CWinSockMulticastWrapper &rmcw : vMCWs )
  {
    rmcw.Disconnect();
  }
  
  for ( std::thread &rtListener : vListeners )
  {
    rtListener.join();
  }

  if ( IsDebuggerPresent() )
  {
    system("pause");
  }
}

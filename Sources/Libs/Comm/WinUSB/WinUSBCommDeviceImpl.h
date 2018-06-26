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
#pragma once

#include <string>
#include <vector>
#include "WinUSBWrapper.h"
#include "ICommDevice.h"

class CWinUSBCommDeviceImpl : public ICommDevice
{
public:
  CWinUSBCommDeviceImpl(void);
  ~CWinUSBCommDeviceImpl(void);

  static BOOL ListDevices(std::vector<std::string> &rNamesList, WORD wVID = 0, WORD wPID = 0);
  BOOL IsConnected() const;
  BOOL ConnectTo(LPCSTR pcszDevice);  ///< Device name from ListDevices
  void Disconnect();

  BOOL SendData(const void *pSource, DWORD dwBufferSizeInBytes);
  BOOL CanReceive(DWORD &rdwNumBytes);
  BOOL DoReceive(void *pDestination, DWORD dwNumBytes);

  DWORD GetMaxBuffer() const { return m_dwCommBufferSizeInBytes; }

public:
  static int ListConnection(unsigned long dwConnection, char * pszDestination, int nMaxAddressLength);
  // Inherited via ICommDevice
  void Connect(const char * pcszWhereTo) override;
  int GetStatus() const override;
  uint32_t Send(const void * pSource, uint32_t dwByteCount) override;
  uint32_t Receive(void * pDestination, uint32_t dwMaxByteCount) override;

  struct SDeviceInfo
  {
    std::string m_strVendor;
    std::string m_strProduct;
    std::string m_strSerial;
    unsigned short m_wVID;
    unsigned short m_wPID;
    std::string GetLongName();
  };

private:
  typedef std::vector<SDeviceInfo> TDeviceList;
  static BOOL listDevices(TDeviceList &rDeviceList, std::vector<std::string> &rPathsList, WORD wVID = 0, WORD wPID = 0);
  static BOOL getPath(LPCSTR pcszDevice, std::string &rstrPath, SDeviceInfo *psDeviceInfo = NULL);

  BOOL queryDeviceEndpoints();
  BOOL controlWrite(BYTE byWinUSBCommControl, BYTE *pbyData = NULL, WORD wNumBytesCount = 0);
  BOOL controlRead(BYTE byWinUSBCommControl, BYTE *pbyData, WORD wNumBytesCount);

  BOOL bulkWrite(const void *pSource, DWORD dwNumBytesCount);
  BOOL bulkRead(void *pDestination, DWORD dwNumBytesCount);

  BOOL reset();
  BYTE readStatus();
  BOOL getResponseLength(DWORD &rdwNumBytes);

  BOOL doSend(const void *pSource, DWORD dwNumBytesCount);

private:
  static GUID sm_WinUSBCommInterfaceGUID;

  HANDLE m_hDeviceHandle;
  WINUSB_INTERFACE_HANDLE m_hWinUSBHandle;
  UCHAR  m_byPipeInId;
  UCHAR  m_byPipeOutId;
  DWORD m_dwCommBufferSizeInBytes;

  BYTE m_byDeviceVersion;

  BYTE m_bycommandReset;
  BYTE m_bycommandGetState;
  BYTE m_bycommandGetBufferSize;
  BYTE m_bycommandGetReturnSize;
};

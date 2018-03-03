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
#ifndef __LINUX_SOCKET_WRAPPER_H__
#define __LINUX_SOCKET_WRAPPER_H__

#include "ICommDevice.h"

class CLinuxSocketWrapper : public ICommDevice
{
public:
  CLinuxSocketWrapper();
  virtual ~CLinuxSocketWrapper() {}
public:
  void StartListening(unsigned short wPort);
  void StopListening();
  void AcceptNewConnection(unsigned long dwTimeout_ms = 100);
  void Connect(const char *pcszDestination, unsigned short wPort);
  void DisconnectClient();
  void Disconnect();

  void ReceiveData(unsigned char *pbyNewData, unsigned long dwNumMaxBytes, unsigned long *pdwNumBytesReceived, unsigned long dwTimeout_ms = 100);
  void SendData(const unsigned char *pbyData, unsigned long dwNumBytes, unsigned long *pdwNumBytesSent);

  bool IsServerOK() const;
  bool IsClientOK() const;

private:
  enum ESocket
  {
    INVALID_SOCKET = -1
  };
  int m_listenSocket = INVALID_SOCKET;
  int m_connectionSocket = INVALID_SOCKET;

public:
  // Inherited via ICommDevice
  int GetStatus() const override;
  void Connect(const char * pcszWhereTo) override;
  void Send(const unsigned char * pbyData, unsigned long dwByteCount, unsigned long * pdwSentByteCount) override;
  void Receive(unsigned char * pbyDestination, unsigned long dwMaxByteCount, unsigned long * pdwHowManyBytes) override;
};

#endif

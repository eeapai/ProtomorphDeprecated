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
#ifndef __WIN_COMM_FACTORY_H__
#define __WIN_COMM_FACTORY_H__
#include "ICommDevice.h"

enum class EWinCommType
{
  unknown,
  WinSockWrapper,
  WinUSBDevice,
  SerialPort,
};

class CWinCommDevice : public ICommDevice
{
public:
  CWinCommDevice() {}
  CWinCommDevice(EWinCommType eOfType) : m_eType(eOfType) {}
  ~CWinCommDevice()
  {
    destroy();
  }

  void SetType(EWinCommType eType);
  static int ListConnection(EWinCommType eOfType, unsigned long dwConnection, char * pszDestination, int nMaxAddressLength);

private:
  void create();
  void destroy();

private:
  EWinCommType m_eType = EWinCommType::unknown;
  ICommDevice * m_pCommDevice = nullptr;

public:
  // Inherited via ICommDevice
  virtual void Connect(const char * pcszWhereTo) override;
  virtual int GetStatus() const override;
  virtual void Send(const void *pSource, unsigned long dwByteCount, unsigned long * pdwSentByteCount) override;
  virtual void Receive(void *pDestination, unsigned long dwMaxByteCount, unsigned long * pdwHowManyBytes) override;
  virtual void Disconnect() override;
};


#endif

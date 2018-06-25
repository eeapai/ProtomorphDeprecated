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

#ifndef __WINUSB_STM32_DEVICE_H__
#define __WINUSB_STM32_DEVICE_H__

#ifdef __cplusplus
 extern "C" {
#endif

typedef enum _EWinUSBSetupResponse
{
  ewinusbsetupresponseOK,
  ewinusbsetupresponseError,
  ewinusbsetupresponseSendToHostOnEP0,
  ewinusbsetupresponseReceiveFromHostOnEP0,
  ewinusbsetupresponseSendToHostOnEPx,
  ewinusbsetupresponseReceiveFromHostOnEPy,
}EWinUSBSetupResponse;
typedef struct _SWinUSBSetupResponse
{
  unsigned long m_dwByteCount;
  unsigned char *m_pbyData;
  EWinUSBSetupResponse m_eResponse;
}SWinUSBSetupResponse;
SWinUSBSetupResponse WinUSBComm_SetupVendorInterface(void *psWinUSBCommSTM32, unsigned char byRequest);
void WinUSBComm_DataIn(void *psWinUSBCommSTM32);
SWinUSBSetupResponse WinUSBComm_EP0_RxReady(void *psWinUSBCommSTM32, unsigned char byRequest);
SWinUSBSetupResponse WinUSBComm_EP0_TxReady(void *psWinUSBCommSTM32, unsigned char byRequest);
void WinUSBComm_DataOut(void *psWinUSBCommSTM32);

#ifdef __cplusplus
}

#include "ICommDevice.h"

///< Type for WinUSB interface data transfer context on STM32
class CWinUSBSTM32Device : public ICommDevice
{
  public:
    CWinUSBSTM32Device(unsigned char *pbyBuffer, unsigned long dwBufferSizeInBytes);
  private:
  unsigned char *m_pbyBuffer;               ///< Pointer to comm buffer
  unsigned long m_dwBufferSizeInBytes;      ///< Size of comm buffer

  unsigned long m_dwExpectedByteCountUSB = 0;   ///< Current incoming packet length

  unsigned long m_dwSendByteCountUSB = 0;       ///< Size of packet to send
  unsigned long m_dwReceivedByteCount = 0;      ///< Size of received packet

  unsigned char m_byStateUSB = 0;               ///< Of EWinUSBComm2State

public:
  void Connect(const char * pcszWhereTo) override;
  int GetStatus() const override;
  void Send(const void *pSource, unsigned long dwByteCount, unsigned long * pdwSentByteCount) override;
  void Receive(void *pDestination, unsigned long dwMaxByteCount, unsigned long * pdwHowManyBytes) override;
  void Disconnect() override;

public:
  SWinUSBSetupResponse SetupVendorInterface(unsigned char byRequest);
  void DataIn();
  SWinUSBSetupResponse EP0_RxReady(unsigned char byRequest);
  SWinUSBSetupResponse EP0_TxReady(unsigned char byRequest);
  void DataOut();
private:
  void updateState();
};

#endif


#endif

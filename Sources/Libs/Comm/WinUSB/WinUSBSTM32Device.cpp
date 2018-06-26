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

#include "WinUSBSTM32Device.h"
#include "WinUSBComm2Shared.h"
#include <string.h>

CWinUSBSTM32Device::CWinUSBSTM32Device(unsigned char *pbyBuffer, unsigned long dwBufferSizeInBytes) :
  m_pbyBuffer(pbyBuffer),
  m_dwBufferSizeInBytes(dwBufferSizeInBytes)
{

}

void CWinUSBSTM32Device::Connect(const char * pcszWhereTo)
{
}

int CWinUSBSTM32Device::GetStatus() const
{
  return ICommDevice::connectionConnected;
}

uint32_t CWinUSBSTM32Device::Send(const void *pSource, uint32_t dwByteCount)
{
  if ( m_dwSendByteCountUSB )
  {
    return 0;
  }
  memmove(m_pbyBuffer, pSource, dwByteCount);
  m_dwSendByteCountUSB = dwByteCount;
  return dwByteCount;
}

uint32_t CWinUSBSTM32Device::Receive(void *pDestination, uint32_t dwMaxByteCount)
{
  if ( m_dwReceivedByteCount )
  {
    memmove(pDestination, m_pbyBuffer, m_dwReceivedByteCount);
    uint32_t dwReceived = m_dwReceivedByteCount;
    m_dwReceivedByteCount = 0;
    return dwReceived;
  }
  return 0;
}

void CWinUSBSTM32Device::Disconnect()
{
}

static unsigned char s_byWinUSBCommVersion = winusbcommversion2;

SWinUSBSetupResponse CWinUSBSTM32Device::SetupVendorInterface(unsigned char byRequest)
{
  SWinUSBSetupResponse response = { 0 };
  response.m_eResponse = ewinusbsetupresponseSendToHostOnEP0; // by defualt
  switch ( byRequest )
  {
  case winusbcomm2commandReset:               response.m_eResponse = ewinusbsetupresponseOK; m_dwExpectedByteCountUSB = 0; m_dwSendByteCountUSB = 0; break;
  case winusbcomm2commandGetVersion:          response.m_pbyData = &s_byWinUSBCommVersion; response.m_dwByteCount = 1; break;
  case winusbcomm2commandGetState:            response.m_pbyData = &m_byStateUSB; response.m_dwByteCount = 1; break;
  case winusbcomm2commandGetBufferSize:       response.m_pbyData = (unsigned char *)(&m_dwBufferSizeInBytes); response.m_dwByteCount = 4; break;
  case winusbcomm2commandGetReturnSize:       response.m_pbyData = (unsigned char *)(&m_dwSendByteCountUSB); response.m_dwByteCount = 4; break;
  case winusbcomm2commandFollowingPacketSize: response.m_eResponse = ewinusbsetupresponseReceiveFromHostOnEP0; m_dwReceivedByteCount = 0; response.m_pbyData = (unsigned char *)(&m_dwExpectedByteCountUSB); response.m_dwByteCount = 4; break;
  default:                                    response.m_eResponse = ewinusbsetupresponseError; break;
  }
  return response;
}

void CWinUSBSTM32Device::DataIn()
{
  m_dwSendByteCountUSB = 0; // is this OK?
  updateState();
}

SWinUSBSetupResponse CWinUSBSTM32Device::EP0_RxReady(unsigned char byRequest)
{
  SWinUSBSetupResponse response = { 0 };
  response.m_eResponse = ewinusbsetupresponseOK;
  if ( winusbcomm2commandFollowingPacketSize == byRequest )
  {
    response.m_eResponse = ewinusbsetupresponseReceiveFromHostOnEPy;
    response.m_pbyData = m_pbyBuffer;
    response.m_dwByteCount = m_dwExpectedByteCountUSB;
  }
  updateState();
  return response;
}

SWinUSBSetupResponse CWinUSBSTM32Device::EP0_TxReady(unsigned char byRequest)
{
  SWinUSBSetupResponse response = { 0 };
  response.m_eResponse = ewinusbsetupresponseOK;
  if ( ( winusbcomm2commandGetReturnSize == byRequest ) && m_dwSendByteCountUSB )
  {
    response.m_eResponse = ewinusbsetupresponseSendToHostOnEPx;
    response.m_pbyData = m_pbyBuffer;
    response.m_dwByteCount = m_dwSendByteCountUSB;
  }
  updateState();
  return response;
}

void CWinUSBSTM32Device::DataOut()
{
  // On STM32F103 this is called after complete transfer data arrived with multiple EP sized packets.
  // USBD_LL_GetRxDataSize returns size of last packet.
  // uint32_t dwNumBytesReceived = USBD_LL_GetRxDataSize(pdev, psWinUSBCommSTM32->m_byEPOutAddress);
  m_dwReceivedByteCount += m_dwExpectedByteCountUSB;
  updateState();
}


void CWinUSBSTM32Device::updateState()
{
  unsigned long dwReceivedByteCountUSB = m_dwReceivedByteCount;

  // If expecting new data
  if ( m_dwExpectedByteCountUSB )
  {
    // It's probably receiving state
    if ( m_dwSendByteCountUSB )
    {
      // But not if data also scheduled to send - error
      m_byStateUSB = winusbcomm2stateError;
      return;
    }
    if ( dwReceivedByteCountUSB < m_dwExpectedByteCountUSB )
    {
      // Still not everything received - receiving
      m_byStateUSB = winusbcomm2stateReceiving;
      return;
    }
    // Everything received - idle
    m_byStateUSB = winusbcomm2stateIdle;
    return;
  }

  // Not expecting any more data
  if ( dwReceivedByteCountUSB )
  {
    // Data remains received
    if ( m_dwSendByteCountUSB )
    {
      // If data scheduled to send - error
      m_byStateUSB = winusbcomm2stateError;
      return;
    }
    // idle
    m_byStateUSB = winusbcomm2stateIdle;
    return;
  }

  // Nothing to receive and nothing received
  if ( m_dwSendByteCountUSB )
  {
    // Data scheduled to send - sending
    m_byStateUSB = winusbcomm2stateSending;
    return;
  }

  // Nothing to receive, nothing received, and nothing to send - idle
  m_byStateUSB = winusbcomm2stateIdle;
}

SWinUSBSetupResponse WinUSBComm_SetupVendorInterface(void *psWinUSBCommSTM32, unsigned char byRequest)
{
  CWinUSBSTM32Device *pCommDev = (CWinUSBSTM32Device *)psWinUSBCommSTM32;
  return pCommDev->SetupVendorInterface(byRequest);
}

void WinUSBComm_DataIn(void *psWinUSBCommSTM32)
{
  CWinUSBSTM32Device *pCommDev = (CWinUSBSTM32Device *)psWinUSBCommSTM32;
  pCommDev->DataIn();
}

SWinUSBSetupResponse WinUSBComm_EP0_RxReady(void *psWinUSBCommSTM32, unsigned char byRequest)
{
  CWinUSBSTM32Device *pCommDev = (CWinUSBSTM32Device *)psWinUSBCommSTM32;
  return pCommDev->EP0_RxReady(byRequest);
}

SWinUSBSetupResponse WinUSBComm_EP0_TxReady(void *psWinUSBCommSTM32, unsigned char byRequest)
{
  CWinUSBSTM32Device *pCommDev = (CWinUSBSTM32Device *)psWinUSBCommSTM32;
  return pCommDev->EP0_TxReady(byRequest);
}

void WinUSBComm_DataOut(void *psWinUSBCommSTM32)
{
  CWinUSBSTM32Device *pCommDev = (CWinUSBSTM32Device *)psWinUSBCommSTM32;
  pCommDev->DataOut();
}

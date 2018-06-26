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
#include "HAL_IO32ICommProxy.h"

#define IO32_COMM_PROXY_ERROR_NO_CONNECTION   -1
#define IO32_COMM_PROXY_ERROR_SEND            -2
#define IO32_COMM_PROXY_ERROR_RECEIVE         -3
#define IO32_COMM_PROXY_ERROR_DIAG_NUM_READS  -4

CHAL_IO32_ICommProxy::CHAL_IO32_ICommProxy(ICommDevice *pCommDevice) :
  m_pCommDevice(pCommDevice)
{
}

int CHAL_IO32_ICommProxy::DoIO(int nNum, const unsigned char * pbyPinsIn, unsigned char * pbyPinsOut, int nNumReads /*= -1*/)
{
  unsigned long dwNumToRead = nNumReads;
  if ( 0 > nNumReads )
  {
    dwNumToRead = IHAL_IO32::NumberOfReads(nNum, pbyPinsIn);
  }
#ifdef _DEBUG
  else
  {
    dwNumToRead = IHAL_IO32::NumberOfReads(nNum, pbyPinsIn);
    if ( dwNumToRead != nNumReads )
    {
      return IO32_COMM_PROXY_ERROR_DIAG_NUM_READS;
    }
  }
#endif

  if ( !IsConnectionOK() )
  {
    return IO32_COMM_PROXY_ERROR_NO_CONNECTION;
  }

  int nNumSent = 0;
  unsigned long dwNumReceived = 0;
  while ( ( nNumSent < nNum ) || ( dwNumReceived < dwNumToRead ) )
  {
    unsigned long dwNumJustSent = SendData(pbyPinsIn + nNumSent, nNum - nNumSent);
    if ( !IsConnectionOK() )
    {
      return IO32_COMM_PROXY_ERROR_SEND;
    }
    nNumSent += dwNumJustSent;

    unsigned long dwNumJustReceived = ReceiveData(pbyPinsOut + dwNumReceived, dwNumToRead - dwNumReceived);
    if ( !IsConnectionOK() )
    {
      return IO32_COMM_PROXY_ERROR_RECEIVE;
    }
    dwNumReceived += dwNumJustReceived;
  }

  return dwNumToRead;
}

bool CHAL_IO32_ICommProxy::IsConnectionOK() const
{
  if ( m_pCommDevice )
  {
    return ICommDevice::connectionConnected == m_pCommDevice->GetStatus();
  }
  return false;
}

uint32_t CHAL_IO32_ICommProxy::ReceiveData(unsigned char * pbyNewData, uint32_t dwNumMaxBytes)
{
  if ( m_pCommDevice && dwNumMaxBytes )
  {
    return m_pCommDevice->Receive(pbyNewData, dwNumMaxBytes);
  }
  return 0;
}

uint32_t CHAL_IO32_ICommProxy::SendData(const unsigned char * pbyData, uint32_t dwNumBytes)
{
  if ( m_pCommDevice && dwNumBytes )
  {
    return m_pCommDevice->Send(pbyData, dwNumBytes);
  }
  return 0;
}


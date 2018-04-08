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
#include "HAL_IO32ICommHost.h"

// #TODO_IO32_ICOMM_HOST Command buffer must handle max comm buffer; handle smaller buffers in Comm infrastructure

CHAL_IO32_ICommHost::CHAL_IO32_ICommHost(IHAL_IO32 *pHAL_IO32, unsigned char *pbyPinsBuff, int nPinsBuffSize, ICommDevice *pComm, const char *pcszConnectionPath) :
  m_pHAL_IO32(pHAL_IO32),
  m_pComm(pComm),
  m_pbyPinsBuff(pbyPinsBuff),
  m_nPinsBuffSize(nPinsBuffSize)
{
  char *pszConnectionPath = m_achConnectionPath;
  if ( pcszConnectionPath )
  {
    unsigned int nGuard = 0;
    while ( (++nGuard < sizeof(m_achConnectionPath)) && *pcszConnectionPath )
    {
      *pszConnectionPath = *pcszConnectionPath;
      pcszConnectionPath++;
      pszConnectionPath++;
    }
  }
  *pszConnectionPath = 0;
}

int CHAL_IO32_ICommHost::HostProcess()
{
  if ( !m_pComm )
  {
    return io32hostNoComm;
  }

  if ( !m_pHAL_IO32 )
  {
    return io32hostNoHAL;
  }

  if ( ICommDevice::connectionDown == m_pComm->GetStatus() )
  {
    m_pComm->Connect(m_achConnectionPath);
    if ( ICommDevice::connectionDown == m_pComm->GetStatus() )
    {
      return io32hostErrorStartingServer;
    }
    else
    {
      return io32hostStartedListening;
    }
  }

  if ( ICommDevice::connectionConnected != m_pComm->GetStatus() )
  {
    m_pComm->Connect(nullptr);
    if ( ICommDevice::connectionConnected != m_pComm->GetStatus() )
    {
      return io32hostWaitingConnection;
    }
    else
    {
      return io32hostClientConnected;
    }
  }

  unsigned long dwNumPinsIn = 0;
  m_pComm->Receive(m_pbyPinsBuff, m_nPinsBuffSize, &dwNumPinsIn);

  if ( ICommDevice::connectionConnected != m_pComm->GetStatus() )
  {
    return io32hostClientDisconnected;
  }

  if ( !dwNumPinsIn )
  {
    return io32hostWaitingDataFromClient;
  }

  int nCountOfReads = m_pHAL_IO32->DoIO(dwNumPinsIn, m_pbyPinsBuff, m_pbyPinsBuff);
  
  if ( 0 > nCountOfReads )
  {
    m_pComm->Disconnect();
    return io32hostHALError;
  }

  int nNumReadsOut = 0;
  while ( nNumReadsOut < nCountOfReads )
  {
    unsigned long dwNumPinsOutCurrent = 0;
    m_pComm->Send(m_pbyPinsBuff + nNumReadsOut, nCountOfReads - nNumReadsOut, &dwNumPinsOutCurrent);
    if ( ICommDevice::connectionConnected != m_pComm->GetStatus() )
    {
      return io32hostUnexpectedClientDisconnect;
    }
    nNumReadsOut += dwNumPinsOutCurrent;
  }

  return io32hostIOCycleDone;
}


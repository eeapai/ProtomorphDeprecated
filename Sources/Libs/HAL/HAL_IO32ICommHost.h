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
#ifndef __HAL_IO32_ICOMM_HOST_H__
#define __HAL_IO32_ICOMM_HOST_H__

#include "ICommDevice.h"
#include "HAL_IO32.h"

class CHAL_IO32_ICommHost
{
public:
  CHAL_IO32_ICommHost(IHAL_IO32 *pHAL_IO32, ICommDevice *pComm, const char *pcszConnectionPath);
public:
  enum EHostState
  {
    io32hostNoComm = 0,
    io32hostNoHAL,
    io32hostErrorStartingServer,
    io32hostStartedListening,
    io32hostWaitingConnection,
    io32hostClientConnected,
    io32hostClientDisconnected,
    io32hostWaitingDataFromClient,
    io32hostHALError,
    io32hostUnexpectedClientDisconnect,
    io32hostIOCycleDone,
  };

  int HostProcess();
private:
  IHAL_IO32 *m_pHAL_IO32 = nullptr;
  ICommDevice *m_pComm = nullptr;
  char m_achConnectionPath[128];
};

#endif


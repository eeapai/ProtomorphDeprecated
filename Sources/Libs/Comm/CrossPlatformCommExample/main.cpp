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

#include <cstdio>

#include <iostream>
#include <string>

#include <thread>
#include <mutex>

#include "ICommDevice.h"

ICommDevice *GetComm();

static void s_printUsage()
{
  printf("xXPCE <[destination:]port>");
}

int main(int argc, char **argv)
{
  ICommDevice *pComm = GetComm();
  std::mutex mCommMutex;
  bool bPauseToSendPlease = false;
  std::thread tSender([&]()
  {
    while (true)
    {
      std::string strInput;
      std::getline(std::cin, strInput);
      bPauseToSendPlease = true;
      mCommMutex.lock();
      if (ICommDevice::connectionConnected == pComm->GetStatus())
      {
        uint32_t dwNumSent = pComm->Send(strInput.c_str(), (uint32_t)strInput.length());
        dwNumSent = dwNumSent; // Ignored. For example purpose work with short strings.
      }
      bPauseToSendPlease = false;
      mCommMutex.unlock();
    }
  });

  if (argc != 2)
  {
    s_printUsage();
    return -1;
  }

  if (!pComm)
  {
    printf("No ICommDevice");
    return -1;
  }

  pComm->Connect(argv[1]);
  ICommDevice::EConnectionStatus eConnStatus = (ICommDevice::EConnectionStatus)pComm->GetStatus();
  if (ICommDevice::connectionDown == eConnStatus)
  {
    std::cout << "Error connecting: " << argv[1] << std::endl;
    return -1;
  }

  ICommDevice::EConnectionStatus eOldConnStatus = ICommDevice::connectionDown;

  while (ICommDevice::connectionDown != eConnStatus)
  {
    if (bPauseToSendPlease)
    {
      continue;
    }
    
    std::lock_guard<std::mutex> guardMutex(mCommMutex);

    if (eOldConnStatus != eConnStatus)
    {
      switch (eConnStatus)
      {
      case ICommDevice::connectionDown: break; // Can't happen
      case ICommDevice::connectionConnected: std::cout << "Connected" << std::endl; break;
      default: std::cout << "Waiting connection" << std::endl; break;
      }
    }
    eOldConnStatus = eConnStatus;

    if (ICommDevice::connectionConnected == eConnStatus)
    {
      // Connection made. Transcive data.
      char achReceived[1024 + 1];
      uint32_t dwNumReceived = pComm->Receive(achReceived, sizeof(achReceived) - 1);
      achReceived[dwNumReceived] = 0;
      if (dwNumReceived)
      {
        std::cout << achReceived << std::endl << std::flush;
      }
    }
    else
    {
      // Try to accept connection.
      pComm->Connect(nullptr);
    }
    eConnStatus = (ICommDevice::EConnectionStatus)pComm->GetStatus();
    if (ICommDevice::connectionDown == eConnStatus)
    {
      std::cout << "Disconnected" << std::endl;
    }
  }
  return 0;
}
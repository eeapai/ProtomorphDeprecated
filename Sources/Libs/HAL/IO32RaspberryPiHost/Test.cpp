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
#include "LinuxSocketWrapper.h"
#include "HAL_IO32LinuxImpl.h"
#include "HAL_IO32LinuxHost.h"
#include <unistd.h> // sleep

void socketWrapperTest()
{
  CLinuxSocketWrapper host;
  CLinuxSocketWrapper client;

  bool bOK = true;
  if ( bOK )
  {
    host.StartListening(1032);
    bOK = host.IsServerOK();
  }
  if ( bOK )
  {
    client.Connect("127.0.0.1", 1032);
    bOK = client.IsClientOK();
  }
  if ( bOK )
  {
    host.AcceptNewConnection();
    bOK = host.IsClientOK();
  }

  unsigned long dwSent = 0;
  if ( bOK )
  {
    client.SendData((unsigned char *)"Hello", 6, &dwSent);
  }

  unsigned char achData[6] = { 0 };
  unsigned long dwReceived = 0;
  if ( bOK )
  {
    host.ReceiveData(achData, 6, &dwReceived);
  }

  if ( bOK )
  {
    client.Disconnect();
  }
  if ( bOK )
  {
    host.Disconnect();
    host.StopListening();
  }
}

void IO32Test()
{
  CHAL_IO32_LinuxImpl io32(1 << 24);
  io32.Initialize();

  unsigned char by24High = IHAL_IO32::pin24 | IHAL_IO32::functionHigh;
  unsigned char by24Low = IHAL_IO32::pin24 | IHAL_IO32::functionLow;
  unsigned char byDummy = 0;
  unsigned long dwTimes = 10;
  while ( dwTimes-- )
  {
    io32.DoIO(1, &by24High, &byDummy);
    //if ( (byDummy & IHAL_IO32::responseResultMask) != IHAL_IO32::responseResultOK )
    //{
    //  break;
    //}
    sleep(1); // s
    io32.DoIO(1, &by24Low, &byDummy);
    //if ( (byDummy & IHAL_IO32::responseResultMask) != IHAL_IO32::responseResultOK )
    //{
    //  break;
    //}
    sleep(1); // s
  }
}

void DoTest()
{
  socketWrapperTest();
  IO32Test();
}           

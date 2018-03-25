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
#include "HAL_IO32ICommHost.h"
#include <cstdio>
#include <poll.h>
#include <unistd.h>

extern void DoTest(); // unused 

static const char *sc_apHostStates[] =
{
  "No Comm",
  "No HAL",
  "Error Starting Server",
  "Started Listening",
  "Waiting Connection",
  "Client Connected",
  "Client Disconnected",
  "Waiting Data From Client",
  "HAL Error",
  "Unexpected Client Disconnect",
  "IO Cycle Done"
};

int main(void)
{
  printf("Initializing GPIO\n");
  CHAL_IO32_LinuxImpl io32(0x0FFFFFFC);
  io32.Initialize();
  printf("Starting IO32 Linux host\n");
  printf("Press ENTER to exit\n");

  CLinuxSocketWrapper server;
  CHAL_IO32_ICommHost io32Host(&io32, &server, "1032");

  bool bRun = true;
  int nHostState = -1;
  while ( bRun )
  {
    int nNewHostState = io32Host.HostProcess();
    if ( nHostState != nNewHostState )
    {
      switch ( nNewHostState )
      {
      case CHAL_IO32_ICommHost::io32hostWaitingDataFromClient:
      case CHAL_IO32_ICommHost::io32hostIOCycleDone:
        break;
      default:
        printf(sc_apHostStates[nNewHostState]);
        printf("\n");
        break;
      }
    }
    nHostState = nNewHostState;
    // See if anything on stdin
    struct pollfd fd2poll;
    fd2poll.fd = STDIN_FILENO;
    fd2poll.events = POLLIN;
    fd2poll.revents = 0;
    if ( 0 < poll(&fd2poll, 1, 0) )
    {
      bRun = false;
    }
  }
  
  int c;
  while ( (c = getchar()) != '\n' && c != EOF ); // flush stdin 
  
  printf("Exiting\n");

  return 0;
}


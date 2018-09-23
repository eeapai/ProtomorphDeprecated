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
#include <stdlib.h>     /* strtol */

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

void printUsage(const char *pcszError = nullptr)
{
  if ( pcszError )
  {
    printf(pcszError);
  }
  printf(
    "\n"
    "IO32RaspberryPiHost [-p<port>] [-m<mask>]\n"
    "\n"
    "  port - TCP/IP listening port\n"
    "         defualt: 1032\n"
    "  mask - allowed pin mask in HEX\n"
    "         mustn't be 0\n"
    "         pin is allowed if bit in the mask at it's position is set\n"
    "           0x00000001 pin0 is allowed\n"
    "           0x00000081 pin7 and pin0 are allowed\n"
    "           0xFFFFFFFF all pins are allowed\n"
    "         default: 0x0FFFFFFC\n"
    "\n"
    "  example: IO32RaspberryPiHost -p1032 -m0x0FFFFFFC\n"
  );
}

static bool s_anythingOnStdIn()
{
  struct pollfd fd2poll;
  fd2poll.fd = STDIN_FILENO;
  fd2poll.events = POLLIN;
  fd2poll.revents = 0;
  return  0 < poll(&fd2poll, 1, 0);
}

int main(int argc, char **argv)
{
  const char *pcszPort = "1032";
  const char *pcszMask = "0x0FFFFFFC";

  for ( int nArg = 1; nArg < argc; ++nArg )
  {
    const char *pcszArg = argv[nArg];
    if ( '-' != pcszArg[0] )
    {
      printf("Bad argument start\n%s\n", pcszArg);
      printUsage();
      return -1;
    }
    switch ( pcszArg[1] )
    {
    case 'p': pcszPort = &pcszArg[2]; break;
    case 'm': pcszMask = &pcszArg[2]; break;
    default:
      printf("Bad argument \n%s\n", pcszArg);
      printUsage();
      return -1;
    }
  }

  uint32_t dwMask = strtol(pcszMask, nullptr, 16);
  if ( !dwMask )
  {
    printUsage("bad mask");
    return -1;
  }
  printf("IO32 host set to listen on port %s; served pin mask 0x%08X\n", pcszPort, dwMask);

  printf("Initializing GPIO\n");
  CHAL_IO32_LinuxImpl io32(0x0FFFFFFC);
  io32.Initialize();
  printf("Starting IO32 Linux host\n");

  CLinuxSocketWrapper server;
  unsigned char abyPinBuff[1024];
  CHAL_IO32_ICommHost io32Host(&io32, abyPinBuff, sizeof(abyPinBuff), &server, pcszPort);

  printf("Press ENTER to exit\n");

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
      case CHAL_IO32_ICommHost::io32hostErrorStartingServer:
        bRun = false; // fall through
      default:
        printf(sc_apHostStates[nNewHostState]);
        printf("\n");
        break;
      }
    }
    nHostState = nNewHostState;
    // See if anything on stdin
    if ( s_anythingOnStdIn() )
    {
      bRun = false;
    }
  }
  
  if ( s_anythingOnStdIn() )
  {
    int c;
    while ( (c = getchar()) != '\n' && c != EOF ); // flush stdin
  }
  
  printf("Exiting\n");

  return 0;
}


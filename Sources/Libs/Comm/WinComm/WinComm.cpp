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

// WinComm.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WinCommFactory.h"
#include "WinSockWrapper.h"
#include "WinUSBCommDeviceImpl.h"
#include "ICommDevice.h"

extern "C"
{
  __declspec(dllexport) ICommDevice * __cdecl CreateCommDevice(EWinCommType eOfType)
  {
    ICommDevice *pCD = nullptr;
    switch ( eOfType )
    {
    case EWinCommType::WinUSBDevice: pCD = new CWinUSBCommDeviceImpl(); break;
    case EWinCommType::WinSockWrapper: pCD = new CWinSockWrapper(); break;
    default: break;
    }
    return pCD;
  }

  __declspec(dllexport) int __cdecl ListConnection(EWinCommType eOfType, unsigned long dwConnection, char * pszDestination, int nMaxAddressLength)
  {
    switch ( eOfType )
    {
    case EWinCommType::WinUSBDevice: return CWinUSBCommDeviceImpl::ListConnection(dwConnection, pszDestination, nMaxAddressLength);
    case EWinCommType::WinSockWrapper: return CWinSockWrapper::ListConnection(dwConnection, pszDestination, nMaxAddressLength);
    default: break;
    }
    return -1;
  }

  __declspec(dllexport) void __cdecl DestroyCommDevice(ICommDevice *pCommDevice)
  {
    delete pCommDevice;
  }
}

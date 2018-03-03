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
#pragma once

#include "SetupAPI.h"

class CSetupAPIWrapper
{
  CSetupAPIWrapper(void);
  ~CSetupAPIWrapper(void);

public:
  static HDEVINFO
    SetupDiGetClassDevsW(
    __in_opt CONST GUID *ClassGuid,
    __in_opt PCWSTR Enumerator,
    __in_opt HWND hwndParent,
    __in DWORD Flags
    );

  static BOOL
    SetupDiEnumDeviceInfo(
    __in HDEVINFO DeviceInfoSet,
    __in DWORD MemberIndex,
    __out PSP_DEVINFO_DATA DeviceInfoData
    );

  static BOOL
    SetupDiEnumDeviceInterfaces(
    __in HDEVINFO DeviceInfoSet,
    __in_opt PSP_DEVINFO_DATA DeviceInfoData,
    __in CONST GUID *InterfaceClassGuid,
    __in DWORD MemberIndex,
    __out PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );

  static BOOL
    SetupDiGetDeviceInterfaceDetailW(
    __in HDEVINFO DeviceInfoSet,
    __in PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    __out_bcount_opt(DeviceInterfaceDetailDataSize) PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData,
    __in DWORD DeviceInterfaceDetailDataSize,
    __out_opt PDWORD RequiredSize, 
    __out_opt PSP_DEVINFO_DATA DeviceInfoData
    );

  static BOOL
    SetupDiDestroyDeviceInfoList(
    __in HDEVINFO DeviceInfoSet
    );

  static BOOL SetupDiGetDeviceRegistryPropertyW(
    __in HDEVINFO DeviceInfoSet,
    __in PSP_DEVINFO_DATA DeviceInfoData,
    __in DWORD Property,
    __out_opt PDWORD PropertyRegDataType,
    __out_bcount_opt(PropertyBufferSize) PBYTE PropertyBuffer,
    __in DWORD PropertyBufferSize,
    __out_opt PDWORD RequiredSize
    );

private:

  //////////////////////////////////////////////////////////////////////////
    typedef HDEVINFO (_stdcall *pfnSetupDiGetClassDevs)(
      __in_opt CONST GUID *ClassGuid,
      __in_opt PCWSTR Enumerator,
      __in_opt HWND hwndParent,
      __in DWORD Flags
      );

    typedef BOOL (_stdcall *pfnSetupDiEnumDeviceInfo)(
      __in HDEVINFO DeviceInfoSet,
      __in DWORD MemberIndex,
      __out PSP_DEVINFO_DATA DeviceInfoData
      );

    typedef BOOL (_stdcall *pfnSetupDiEnumDeviceInterfaces)(
      __in HDEVINFO DeviceInfoSet,
      __in_opt PSP_DEVINFO_DATA DeviceInfoData,
      __in CONST GUID *InterfaceClassGuid,
      __in DWORD MemberIndex,
      __out PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
      );

    typedef BOOL (_stdcall *pfnSetupDiGetDeviceInterfaceDetail)(
      __in HDEVINFO DeviceInfoSet,
      __in PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
      __out_bcount_opt(DeviceInterfaceDetailDataSize) PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData,
      __in DWORD DeviceInterfaceDetailDataSize,
      __out_opt PDWORD RequiredSize, 
      __out_opt PSP_DEVINFO_DATA DeviceInfoData
      );

    typedef BOOL (_stdcall *pfnSetupDiDestroyDeviceInfoList)(
      __in HDEVINFO DeviceInfoSet
      );

    typedef BOOL (_stdcall *pfnSetupDiGetDeviceRegistryProperty)(
      __in HDEVINFO DeviceInfoSet,
      __in PSP_DEVINFO_DATA DeviceInfoData,
      __in DWORD Property,
      __out_opt PDWORD PropertyRegDataType,
      __out_bcount_opt(PropertyBufferSize) PBYTE PropertyBuffer,
      __in DWORD PropertyBufferSize,
      __out_opt PDWORD RequiredSize
      );


  //////////////////////////////////////////////////////////////////////////

  static pfnSetupDiGetClassDevs sm_pfnSetupDiGetClassDevs;
  static pfnSetupDiEnumDeviceInfo sm_pfnSetupDiEnumDeviceInfo;
  static pfnSetupDiEnumDeviceInterfaces sm_pfnSetupDiEnumDeviceInterfaces;
  static pfnSetupDiGetDeviceInterfaceDetail sm_pfnSetupDiGetDeviceInterfaceDetail;
  static pfnSetupDiDestroyDeviceInfoList sm_pfnSetupDiDestroyDeviceInfoList;
  static pfnSetupDiGetDeviceRegistryProperty sm_pfnSetupDiGetDeviceRegistryProperty;

  static CSetupAPIWrapper sm_SetupAPIWrapper;
  static HMODULE sm_hSetupAPI;
};

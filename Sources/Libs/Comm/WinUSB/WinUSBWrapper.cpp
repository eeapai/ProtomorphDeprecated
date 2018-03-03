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
#include "StdAfx.h"
#include "WinUSBWrapper.h"

CWinUSBWrapper CWinUSBWrapper::sm_WinUSBWrapper;
HMODULE CWinUSBWrapper::sm_hWinUSB;

CWinUSBWrapper::pfnWinUsb_Initialize                      CWinUSBWrapper::m_pfnInitialize = NULL;
CWinUSBWrapper::pfnWinUsb_Free                            CWinUSBWrapper::m_pfnFree = NULL;
CWinUSBWrapper::pfnWinUsb_GetAssociatedInterface          CWinUSBWrapper::m_pfnGetAssociatedInterface = NULL;
CWinUSBWrapper::pfnWinUsb_GetDescriptor                   CWinUSBWrapper::m_pfnGetDescriptor = NULL;
CWinUSBWrapper::pfnWinUsb_QueryInterfaceSettings          CWinUSBWrapper::m_pfnQueryInterfaceSettings = NULL;
CWinUSBWrapper::pfnWinUsb_QueryDeviceInformation          CWinUSBWrapper::m_pfnQueryDeviceInformation = NULL;
CWinUSBWrapper::pfnWinUsb_SetCurrentAlternateSetting      CWinUSBWrapper::m_pfnSetCurrentAlternateSetting = NULL;
CWinUSBWrapper::pfnWinUsb_GetCurrentAlternateSetting      CWinUSBWrapper::m_pfnGetCurrentAlternateSetting = NULL;
CWinUSBWrapper::pfnWinUsb_QueryPipe                       CWinUSBWrapper::m_pfnQueryPipe = NULL;
CWinUSBWrapper::pfnWinUsb_SetPipePolicy                   CWinUSBWrapper::m_pfnSetPipePolicy = NULL;
CWinUSBWrapper::pfnWinUsb_GetPipePolicy                   CWinUSBWrapper::m_pfnGetPipePolicy = NULL;
CWinUSBWrapper::pfnWinUsb_ReadPipe                        CWinUSBWrapper::m_pfnReadPipe = NULL;
CWinUSBWrapper::pfnWinUsb_WritePipe                       CWinUSBWrapper::m_pfnWritePipe = NULL;
CWinUSBWrapper::pfnWinUsb_ControlTransfer                 CWinUSBWrapper::m_pfnControlTransfer = NULL;
CWinUSBWrapper::pfnWinUsb_ResetPipe                       CWinUSBWrapper::m_pfnResetPipe = NULL;
CWinUSBWrapper::pfnWinUsb_AbortPipe                       CWinUSBWrapper::m_pfnAbortPipe = NULL;
CWinUSBWrapper::pfnWinUsb_FlushPipe                       CWinUSBWrapper::m_pfnFlushPipe = NULL;
CWinUSBWrapper::pfnWinUsb_SetPowerPolicy                  CWinUSBWrapper::m_pfnSetPowerPolicy = NULL;
CWinUSBWrapper::pfnWinUsb_GetPowerPolicy                  CWinUSBWrapper::m_pfnGetPowerPolicy = NULL;
CWinUSBWrapper::pfnWinUsb_GetOverlappedResult             CWinUSBWrapper::m_pfnGetOverlappedResult = NULL;
CWinUSBWrapper::pfnWinUsb_ParseConfigurationDescriptor    CWinUSBWrapper::m_pfnParseConfigurationDescriptor = NULL;
CWinUSBWrapper::pfnWinUsb_ParseDescriptors                CWinUSBWrapper::m_pfnParseDescriptors = NULL;


CWinUSBWrapper::CWinUSBWrapper(void)
{
  if ( NULL == sm_hWinUSB )
  {
    sm_hWinUSB = ::LoadLibrary(_TPARAM("winusb.dll"));

    if (NULL == sm_hWinUSB)
    {
      return;
    }

    m_pfnInitialize                     = (pfnWinUsb_Initialize                      )::GetProcAddress(sm_hWinUSB, "WinUsb_Initialize");
    m_pfnFree                           = (pfnWinUsb_Free                            )::GetProcAddress(sm_hWinUSB, "WinUsb_Free");
    m_pfnGetAssociatedInterface         = (pfnWinUsb_GetAssociatedInterface          )::GetProcAddress(sm_hWinUSB, "WinUsb_GetAssociatedInterface");
    m_pfnGetDescriptor                  = (pfnWinUsb_GetDescriptor                   )::GetProcAddress(sm_hWinUSB, "WinUsb_GetDescriptor");
    m_pfnQueryInterfaceSettings         = (pfnWinUsb_QueryInterfaceSettings          )::GetProcAddress(sm_hWinUSB, "WinUsb_QueryInterfaceSettings");
    m_pfnQueryDeviceInformation         = (pfnWinUsb_QueryDeviceInformation          )::GetProcAddress(sm_hWinUSB, "WinUsb_QueryDeviceInformation");
    m_pfnSetCurrentAlternateSetting     = (pfnWinUsb_SetCurrentAlternateSetting      )::GetProcAddress(sm_hWinUSB, "WinUsb_SetCurrentAlternateSetting");
    m_pfnGetCurrentAlternateSetting     = (pfnWinUsb_GetCurrentAlternateSetting      )::GetProcAddress(sm_hWinUSB, "WinUsb_GetCurrentAlternateSetting");
    m_pfnQueryPipe                      = (pfnWinUsb_QueryPipe                       )::GetProcAddress(sm_hWinUSB, "WinUsb_QueryPipe");
    m_pfnSetPipePolicy                  = (pfnWinUsb_SetPipePolicy                   )::GetProcAddress(sm_hWinUSB, "WinUsb_SetPipePolicy");
    m_pfnGetPipePolicy                  = (pfnWinUsb_GetPipePolicy                   )::GetProcAddress(sm_hWinUSB, "WinUsb_GetPipePolicy");
    m_pfnReadPipe                       = (pfnWinUsb_ReadPipe                        )::GetProcAddress(sm_hWinUSB, "WinUsb_ReadPipe");
    m_pfnWritePipe                      = (pfnWinUsb_WritePipe                       )::GetProcAddress(sm_hWinUSB, "WinUsb_WritePipe");
    m_pfnControlTransfer                = (pfnWinUsb_ControlTransfer                 )::GetProcAddress(sm_hWinUSB, "WinUsb_ControlTransfer");
    m_pfnResetPipe                      = (pfnWinUsb_ResetPipe                       )::GetProcAddress(sm_hWinUSB, "WinUsb_ResetPipe");
    m_pfnAbortPipe                      = (pfnWinUsb_AbortPipe                       )::GetProcAddress(sm_hWinUSB, "WinUsb_AbortPipe");
    m_pfnFlushPipe                      = (pfnWinUsb_FlushPipe                       )::GetProcAddress(sm_hWinUSB, "WinUsb_FlushPipe");
    m_pfnSetPowerPolicy                 = (pfnWinUsb_SetPowerPolicy                  )::GetProcAddress(sm_hWinUSB, "WinUsb_SetPowerPolicy");
    m_pfnGetPowerPolicy                 = (pfnWinUsb_GetPowerPolicy                  )::GetProcAddress(sm_hWinUSB, "WinUsb_GetPowerPolicy");
    m_pfnGetOverlappedResult            = (pfnWinUsb_GetOverlappedResult             )::GetProcAddress(sm_hWinUSB, "WinUsb_GetOverlappedResult");
    m_pfnParseConfigurationDescriptor   = (pfnWinUsb_ParseConfigurationDescriptor    )::GetProcAddress(sm_hWinUSB, "WinUsb_ParseConfigurationDescriptor");
    m_pfnParseDescriptors               = (pfnWinUsb_ParseDescriptors                )::GetProcAddress(sm_hWinUSB, "WinUsb_ParseDescriptors");

    BOOL bResult = TRUE;

    if ( NULL == m_pfnInitialize ) { bResult = FALSE; }
    if ( NULL == m_pfnFree ) { bResult = FALSE; }
    if ( NULL == m_pfnGetAssociatedInterface ) { bResult = FALSE; }
    if ( NULL == m_pfnGetDescriptor ) { bResult = FALSE; }
    if ( NULL == m_pfnQueryInterfaceSettings ) { bResult = FALSE; }
    if ( NULL == m_pfnQueryDeviceInformation ) { bResult = FALSE; }
    if ( NULL == m_pfnSetCurrentAlternateSetting ) { bResult = FALSE; }
    if ( NULL == m_pfnGetCurrentAlternateSetting ) { bResult = FALSE; }
    if ( NULL == m_pfnQueryPipe ) { bResult = FALSE; }
    if ( NULL == m_pfnSetPipePolicy ) { bResult = FALSE; }
    if ( NULL == m_pfnGetPipePolicy ) { bResult = FALSE; }
    if ( NULL == m_pfnReadPipe ) { bResult = FALSE; }
    if ( NULL == m_pfnWritePipe ) { bResult = FALSE; }
    if ( NULL == m_pfnControlTransfer ) { bResult = FALSE; }
    if ( NULL == m_pfnResetPipe ) { bResult = FALSE; }
    if ( NULL == m_pfnAbortPipe ) { bResult = FALSE; }
    if ( NULL == m_pfnFlushPipe ) { bResult = FALSE; }
    if ( NULL == m_pfnSetPowerPolicy ) { bResult = FALSE; }
    if ( NULL == m_pfnGetPowerPolicy ) { bResult = FALSE; }
    if ( NULL == m_pfnGetOverlappedResult ) { bResult = FALSE; }
    if ( NULL == m_pfnParseConfigurationDescriptor ) { bResult = FALSE; }
    if ( NULL == m_pfnParseDescriptors ) { bResult = FALSE; }

    if ( !bResult )
    {
      ::FreeLibrary(sm_hWinUSB);
      sm_hWinUSB = NULL;
    }
  }

}

CWinUSBWrapper::~CWinUSBWrapper(void)
{
  if ( sm_hWinUSB )
  {
    ::FreeLibrary(sm_hWinUSB);
    sm_hWinUSB = NULL;
  }
}

BOOL CWinUSBWrapper::Initialize(
                __in  HANDLE DeviceHandle,
                __out PWINUSB_INTERFACE_HANDLE InterfaceHandle
                )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult =  m_pfnInitialize(DeviceHandle, InterfaceHandle);
  return bResult;
}


BOOL CWinUSBWrapper::Free(
          __in  WINUSB_INTERFACE_HANDLE InterfaceHandle
          )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnFree(InterfaceHandle);
  return bResult;
}


BOOL CWinUSBWrapper::GetAssociatedInterface(
                            __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                            __in  UCHAR AssociatedInterfaceIndex,
                            __out PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
                            )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetAssociatedInterface(InterfaceHandle, AssociatedInterfaceIndex, AssociatedInterfaceHandle);
  return bResult;
}



BOOL CWinUSBWrapper::GetDescriptor(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR DescriptorType,
                   __in  UCHAR Index,
                   __in  USHORT LanguageID,
                   __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
                   __in  ULONG BufferLength,
                   __out PULONG LengthTransferred
                   )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetDescriptor(InterfaceHandle, DescriptorType, Index, LanguageID, Buffer, BufferLength, LengthTransferred);
  return bResult;
}

BOOL CWinUSBWrapper::QueryInterfaceSettings(
                            __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                            __in  UCHAR AlternateInterfaceNumber,
                            __out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
                            )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnQueryInterfaceSettings(InterfaceHandle, AlternateInterfaceNumber, UsbAltInterfaceDescriptor);
  return bResult;
}

BOOL CWinUSBWrapper::QueryDeviceInformation(
                            __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                            __in  ULONG InformationType,
                            __inout PULONG BufferLength,
                            __out_bcount(*BufferLength) PVOID Buffer
                            )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnQueryDeviceInformation(InterfaceHandle, InformationType, BufferLength, Buffer);
  return bResult;
}

BOOL CWinUSBWrapper::SetCurrentAlternateSetting(
                                __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                __in  UCHAR SettingNumber
                                )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnSetCurrentAlternateSetting(InterfaceHandle, SettingNumber);
  return bResult;
}

BOOL CWinUSBWrapper::GetCurrentAlternateSetting(
                                __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                __out PUCHAR SettingNumber
                                )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetCurrentAlternateSetting(InterfaceHandle, SettingNumber);
  return bResult;
}


BOOL CWinUSBWrapper::QueryPipe(
               __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
               __in  UCHAR AlternateInterfaceNumber,
               __in  UCHAR PipeIndex,
               __out PWINUSB_PIPE_INFORMATION PipeInformation
               )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnQueryPipe(InterfaceHandle, AlternateInterfaceNumber, PipeIndex, PipeInformation);
  return bResult;
}


BOOL CWinUSBWrapper::SetPipePolicy(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID,
                   __in  ULONG PolicyType,
                   __in  ULONG ValueLength,
                   __in_bcount(ValueLength) PVOID Value
                   )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnSetPipePolicy(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
  return bResult;
}

BOOL CWinUSBWrapper::GetPipePolicy(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID,
                   __in  ULONG PolicyType,
                   __inout PULONG ValueLength,
                   __out_bcount(*ValueLength) PVOID Value
                   )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetPipePolicy(InterfaceHandle, PipeID, PolicyType, ValueLength, Value);
  return bResult;
}

BOOL CWinUSBWrapper::ReadPipe(
              __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
              __in  UCHAR PipeID,
              __out_bcount_part_opt(BufferLength,*LengthTransferred) PUCHAR Buffer,
              __in  ULONG BufferLength,
              __out_opt PULONG LengthTransferred,
              __in_opt LPOVERLAPPED Overlapped
              )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnReadPipe(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped);
  return bResult;
}

BOOL CWinUSBWrapper::WritePipe(
               __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
               __in  UCHAR PipeID,
               __in_bcount(BufferLength) PUCHAR Buffer,
               __in  ULONG BufferLength,
               __out_opt PULONG LengthTransferred,
               __in_opt LPOVERLAPPED Overlapped    
               )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnWritePipe(InterfaceHandle, PipeID, Buffer, BufferLength, LengthTransferred, Overlapped);
  return bResult;
}

BOOL CWinUSBWrapper::ControlTransfer(
                     __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                     __in  WINUSB_SETUP_PACKET SetupPacket,
                     __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
                     __in  ULONG BufferLength,
                     __out_opt PULONG LengthTransferred,
                     __in_opt  LPOVERLAPPED Overlapped    
                     )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnControlTransfer(InterfaceHandle, SetupPacket, Buffer, BufferLength, LengthTransferred, Overlapped);
  return bResult;
}

BOOL CWinUSBWrapper::ResetPipe(
               __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
               __in  UCHAR PipeID
               )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnResetPipe(InterfaceHandle, PipeID);
  return bResult;
}

BOOL CWinUSBWrapper::AbortPipe(
               __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
               __in  UCHAR PipeID
               )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnAbortPipe(InterfaceHandle, PipeID);
  return bResult;
}

BOOL CWinUSBWrapper::FlushPipe(
               __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
               __in  UCHAR PipeID
               )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnFlushPipe(InterfaceHandle, PipeID);
  return bResult;
}

BOOL CWinUSBWrapper::SetPowerPolicy(
                    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                    __in  ULONG PolicyType,
                    __in  ULONG ValueLength,
                    __in_bcount(ValueLength) PVOID Value
                    )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnSetPowerPolicy(InterfaceHandle, PolicyType, ValueLength, Value);
  return bResult;
}

BOOL CWinUSBWrapper::GetPowerPolicy(
                    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                    __in  ULONG PolicyType,
                    __inout PULONG ValueLength,
                    __out_bcount(*ValueLength) PVOID Value
                    )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetPowerPolicy(InterfaceHandle, PolicyType, ValueLength, Value);
  return bResult;
}

BOOL CWinUSBWrapper::GetOverlappedResult(
                         __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                         __in  LPOVERLAPPED lpOverlapped,
                         __out LPDWORD lpNumberOfBytesTransferred,
                         __in  BOOL bWait
                         )
{
  if ( !sm_hWinUSB )
  {
    return FALSE;
  }
  BOOL bResult = m_pfnGetOverlappedResult(InterfaceHandle, lpOverlapped, lpNumberOfBytesTransferred, bWait);
  return bResult;
}


PUSB_INTERFACE_DESCRIPTOR CWinUSBWrapper::ParseConfigurationDescriptor(
  __in  PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
  __in  PVOID StartPosition,
  __in  LONG InterfaceNumber,
  __in  LONG AlternateSetting,
  __in  LONG InterfaceClass,
  __in  LONG InterfaceSubClass,
  __in  LONG InterfaceProtocol
  )
{
  if ( !sm_hWinUSB )
  {
    return NULL;
  }
  return m_pfnParseConfigurationDescriptor(ConfigurationDescriptor, StartPosition, InterfaceNumber, AlternateSetting, InterfaceClass, InterfaceSubClass, InterfaceProtocol);
}

PUSB_COMMON_DESCRIPTOR CWinUSBWrapper::ParseDescriptors(
  __in_bcount(TotalLength) PVOID    DescriptorBuffer,
  __in  ULONG    TotalLength,
  __in  PVOID    StartPosition,
  __in  LONG     DescriptorType
  )
{
  if ( !sm_hWinUSB )
  {
    return NULL;
  }
  return m_pfnParseDescriptors(DescriptorBuffer, TotalLength, StartPosition, DescriptorType);
}

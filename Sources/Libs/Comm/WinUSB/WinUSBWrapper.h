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

#include "winusb.h"

class CWinUSBWrapper
{
  CWinUSBWrapper(void);
  ~CWinUSBWrapper(void);
public:

  static BOOL Initialize(
    __in  HANDLE DeviceHandle,
    __out PWINUSB_INTERFACE_HANDLE InterfaceHandle
    );


  static BOOL Free(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle
    );


  static BOOL GetAssociatedInterface(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR AssociatedInterfaceIndex,
    __out PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
    );



  static BOOL GetDescriptor(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR DescriptorType,
    __in  UCHAR Index,
    __in  USHORT LanguageID,
    __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
    __in  ULONG BufferLength,
    __out PULONG LengthTransferred
    );

  static BOOL QueryInterfaceSettings(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR AlternateInterfaceNumber,
    __out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
    );

  static BOOL QueryDeviceInformation(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  ULONG InformationType,
    __inout PULONG BufferLength,
    __out_bcount(*BufferLength) PVOID Buffer
    );

  static BOOL SetCurrentAlternateSetting(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR SettingNumber
    );

  static BOOL GetCurrentAlternateSetting(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __out PUCHAR SettingNumber
    );


  static BOOL QueryPipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR AlternateInterfaceNumber,
    __in  UCHAR PipeIndex,
    __out PWINUSB_PIPE_INFORMATION PipeInformation
    );


  static BOOL SetPipePolicy(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID,
    __in  ULONG PolicyType,
    __in  ULONG ValueLength,
    __in_bcount(ValueLength) PVOID Value
    );

  static BOOL GetPipePolicy(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID,
    __in  ULONG PolicyType,
    __inout PULONG ValueLength,
    __out_bcount(*ValueLength) PVOID Value
    );

  static BOOL ReadPipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID,
    __out_bcount_part_opt(BufferLength,*LengthTransferred) PUCHAR Buffer,
    __in  ULONG BufferLength,
    __out_opt PULONG LengthTransferred,
    __in_opt LPOVERLAPPED Overlapped
    );

  static BOOL WritePipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID,
    __in_bcount(BufferLength) PUCHAR Buffer,
    __in  ULONG BufferLength,
    __out_opt PULONG LengthTransferred,
    __in_opt LPOVERLAPPED Overlapped    
    );

  static BOOL ControlTransfer(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  WINUSB_SETUP_PACKET SetupPacket,
    __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
    __in  ULONG BufferLength,
    __out_opt PULONG LengthTransferred,
    __in_opt  LPOVERLAPPED Overlapped    
    );

  static BOOL ResetPipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID
    );

  static BOOL AbortPipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID
    );

  static BOOL FlushPipe(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  UCHAR PipeID
    );

  static BOOL SetPowerPolicy(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  ULONG PolicyType,
    __in  ULONG ValueLength,
    __in_bcount(ValueLength) PVOID Value
    );

  static BOOL GetPowerPolicy(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  ULONG PolicyType,
    __inout PULONG ValueLength,
    __out_bcount(*ValueLength) PVOID Value
    );

  static BOOL GetOverlappedResult(
    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
    __in  LPOVERLAPPED lpOverlapped,
    __out LPDWORD lpNumberOfBytesTransferred,
    __in  BOOL bWait
    );


  static PUSB_INTERFACE_DESCRIPTOR ParseConfigurationDescriptor(
    __in  PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    __in  PVOID StartPosition,
    __in  LONG InterfaceNumber,
    __in  LONG AlternateSetting,
    __in  LONG InterfaceClass,
    __in  LONG InterfaceSubClass,
    __in  LONG InterfaceProtocol
    );

  static PUSB_COMMON_DESCRIPTOR ParseDescriptors(
    __in_bcount(TotalLength) PVOID    DescriptorBuffer,
    __in  ULONG    TotalLength,
    __in  PVOID    StartPosition,
    __in  LONG     DescriptorType
    );

private:
  typedef BOOL 
  (_stdcall *pfnWinUsb_Initialize)(
                    __in  HANDLE DeviceHandle,
                    __out PWINUSB_INTERFACE_HANDLE InterfaceHandle
                    );


  typedef BOOL
  (_stdcall *pfnWinUsb_Free)(
              __in  WINUSB_INTERFACE_HANDLE InterfaceHandle
              );


  typedef BOOL
  (_stdcall *pfnWinUsb_GetAssociatedInterface)(
                                __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                __in  UCHAR AssociatedInterfaceIndex,
                                __out PWINUSB_INTERFACE_HANDLE AssociatedInterfaceHandle
                                );



  typedef BOOL
  (_stdcall *pfnWinUsb_GetDescriptor)(
                       __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                       __in  UCHAR DescriptorType,
                       __in  UCHAR Index,
                       __in  USHORT LanguageID,
                       __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
                       __in  ULONG BufferLength,
                       __out PULONG LengthTransferred
                       );

  typedef BOOL
  (_stdcall *pfnWinUsb_QueryInterfaceSettings)(
                                __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                __in  UCHAR AlternateInterfaceNumber,
                                __out PUSB_INTERFACE_DESCRIPTOR UsbAltInterfaceDescriptor
                                );

  typedef BOOL
  (_stdcall *pfnWinUsb_QueryDeviceInformation)(
                                __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                __in  ULONG InformationType,
                                __inout PULONG BufferLength,
                                __out_bcount(*BufferLength) PVOID Buffer
                                );

  typedef BOOL
  (_stdcall *pfnWinUsb_SetCurrentAlternateSetting)(
                                    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                    __in  UCHAR SettingNumber
                                    );

  typedef BOOL
  (_stdcall *pfnWinUsb_GetCurrentAlternateSetting)(
                                    __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                                    __out PUCHAR SettingNumber
                                    );


  typedef BOOL
  (_stdcall *pfnWinUsb_QueryPipe)(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR AlternateInterfaceNumber,
                   __in  UCHAR PipeIndex,
                   __out PWINUSB_PIPE_INFORMATION PipeInformation
                   );


  typedef BOOL
  (_stdcall *pfnWinUsb_SetPipePolicy)(
                       __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                       __in  UCHAR PipeID,
                       __in  ULONG PolicyType,
                       __in  ULONG ValueLength,
                       __in_bcount(ValueLength) PVOID Value
                       );

  typedef BOOL
  (_stdcall *pfnWinUsb_GetPipePolicy)(
                       __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                       __in  UCHAR PipeID,
                       __in  ULONG PolicyType,
                       __inout PULONG ValueLength,
                       __out_bcount(*ValueLength) PVOID Value
                       );

  typedef BOOL
  (_stdcall *pfnWinUsb_ReadPipe)(
                  __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                  __in  UCHAR PipeID,
                  __out_bcount_part_opt(BufferLength,*LengthTransferred) PUCHAR Buffer,
                  __in  ULONG BufferLength,
                  __out_opt PULONG LengthTransferred,
                  __in_opt LPOVERLAPPED Overlapped
                  );

  typedef BOOL
  (_stdcall *pfnWinUsb_WritePipe)(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID,
                   __in_bcount(BufferLength) PUCHAR Buffer,
                   __in  ULONG BufferLength,
                   __out_opt PULONG LengthTransferred,
                   __in_opt LPOVERLAPPED Overlapped    
                   );

  typedef BOOL
  (_stdcall *pfnWinUsb_ControlTransfer)(
                         __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                         __in  WINUSB_SETUP_PACKET SetupPacket,
                         __out_bcount_part_opt(BufferLength, *LengthTransferred) PUCHAR Buffer,
                         __in  ULONG BufferLength,
                         __out_opt PULONG LengthTransferred,
                         __in_opt  LPOVERLAPPED Overlapped    
                         );

  typedef BOOL
  (_stdcall *pfnWinUsb_ResetPipe)(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID
                   );

  typedef BOOL
  (_stdcall *pfnWinUsb_AbortPipe)(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID
                   );

  typedef BOOL
  (_stdcall *pfnWinUsb_FlushPipe)(
                   __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                   __in  UCHAR PipeID
                   );

  typedef BOOL
  (_stdcall *pfnWinUsb_SetPowerPolicy)(
                        __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                        __in  ULONG PolicyType,
                        __in  ULONG ValueLength,
                        __in_bcount(ValueLength) PVOID Value
                        );

  typedef BOOL
  (_stdcall *pfnWinUsb_GetPowerPolicy)(
                        __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                        __in  ULONG PolicyType,
                        __inout PULONG ValueLength,
                        __out_bcount(*ValueLength) PVOID Value
                        );

  typedef BOOL
  (_stdcall *pfnWinUsb_GetOverlappedResult)(
                             __in  WINUSB_INTERFACE_HANDLE InterfaceHandle,
                             __in  LPOVERLAPPED lpOverlapped,
                             __out LPDWORD lpNumberOfBytesTransferred,
                             __in  BOOL bWait
                             );


  typedef PUSB_INTERFACE_DESCRIPTOR
  (_stdcall *pfnWinUsb_ParseConfigurationDescriptor)(
                                      __in  PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
                                      __in  PVOID StartPosition,
                                      __in  LONG InterfaceNumber,
                                      __in  LONG AlternateSetting,
                                      __in  LONG InterfaceClass,
                                      __in  LONG InterfaceSubClass,
                                      __in  LONG InterfaceProtocol
                                      );

  typedef PUSB_COMMON_DESCRIPTOR
  (_stdcall *pfnWinUsb_ParseDescriptors)(
                          __in_bcount(TotalLength) PVOID    DescriptorBuffer,
                          __in  ULONG    TotalLength,
                          __in  PVOID    StartPosition,
                          __in  LONG     DescriptorType
                          );

  static pfnWinUsb_Initialize                      m_pfnInitialize;
  static pfnWinUsb_Free                            m_pfnFree;
  static pfnWinUsb_GetAssociatedInterface          m_pfnGetAssociatedInterface;
  static pfnWinUsb_GetDescriptor                   m_pfnGetDescriptor;
  static pfnWinUsb_QueryInterfaceSettings          m_pfnQueryInterfaceSettings;
  static pfnWinUsb_QueryDeviceInformation          m_pfnQueryDeviceInformation;
  static pfnWinUsb_SetCurrentAlternateSetting      m_pfnSetCurrentAlternateSetting;
  static pfnWinUsb_GetCurrentAlternateSetting      m_pfnGetCurrentAlternateSetting;
  static pfnWinUsb_QueryPipe                       m_pfnQueryPipe;
  static pfnWinUsb_SetPipePolicy                   m_pfnSetPipePolicy;
  static pfnWinUsb_GetPipePolicy                   m_pfnGetPipePolicy;
  static pfnWinUsb_ReadPipe                        m_pfnReadPipe;
  static pfnWinUsb_WritePipe                       m_pfnWritePipe;
  static pfnWinUsb_ControlTransfer                 m_pfnControlTransfer;
  static pfnWinUsb_ResetPipe                       m_pfnResetPipe;
  static pfnWinUsb_AbortPipe                       m_pfnAbortPipe;
  static pfnWinUsb_FlushPipe                       m_pfnFlushPipe;
  static pfnWinUsb_SetPowerPolicy                  m_pfnSetPowerPolicy;
  static pfnWinUsb_GetPowerPolicy                  m_pfnGetPowerPolicy;
  static pfnWinUsb_GetOverlappedResult             m_pfnGetOverlappedResult;
  static pfnWinUsb_ParseConfigurationDescriptor    m_pfnParseConfigurationDescriptor;
  static pfnWinUsb_ParseDescriptors                m_pfnParseDescriptors;

  static CWinUSBWrapper sm_WinUSBWrapper;
  static HMODULE sm_hWinUSB;
};

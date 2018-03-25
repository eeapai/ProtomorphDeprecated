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
#include "usbd_io32.h"
#include "usbd_core.h"
#include "WinUSBSTM32Device.h"
#include "Defs.h"

#if 0
#define USE_MS_OS_DESC_10 true // not tested for a long time
#endif

#ifdef USE_MS_OS_DESC_10
#define MS_VendorCode 'P'
#endif

#define MS_VendorCode2 'W'
#define WU_VendorCode 'w'


#define IO32_MAX_EP_PACKET            0x40

#define USB_IO32_CONFIG_DESC_HEADER_SIZE 9
#define USB_IO32_CONFIG_DESC_SIZ       (9 + 9+7+7)
#define USB_IO32_COMPAT_ID_OS_DESC_SIZ       (16 + 24)

#define USB_DESC_W(w) ((w) & 0xFF), ((w) >> 8)

static /*const*/ uint8_t s_abyUSBD_IO32_ConfigurationDescriptor[USB_IO32_CONFIG_DESC_SIZ] =
{
  USB_IO32_CONFIG_DESC_HEADER_SIZE,     // bLength
  USB_DESC_TYPE_CONFIGURATION,          // bDescriptorType
  USB_DESC_W(USB_IO32_CONFIG_DESC_SIZ), // wTotalLength
  io32stm32usbinterface_NumInterfaces,  // bNumInterfaces
  0x01,                                 // bConfigurationValue
  USBD_IDX_CONFIG_STR,                  // iConfiguration
  0x80,                                 // bmAttributes
  0xFA,                                 // bMaxPower 500 mA
  //// IO32 interface over WinUSB
  0x09,   // bLength: Interface Descriptor size
  USB_DESC_TYPE_INTERFACE,   // bDescriptorType
  io32stm32usbinterface_WinUSBComm,   // bInterfaceNumber
  0x00,   // bAlternateSetting
  0x02,   // bNumEndpoints
  0xFF,   // bInterfaceClass: Vendor
  0x00,   // bInterfaceSubClass: none
  0x00,   // nInterfaceProtocol
  USBD_IDX_INTERFACE_STR, // iInterface
  // Endpoints for IO32 interface over WinUSB
  0x07,   // bLength: Endpoint descriptor pwLength
  USB_DESC_TYPE_ENDPOINT,   // bDescriptorType
  WINUSBCOMM_IO32EPIN_ADDR,   // bEndpointAddress
  USBD_EP_TYPE_BULK,   // bmAttributes
  USB_DESC_W(IO32_MAX_EP_PACKET), // wMaxPacketSize
  0x00,   // bInterval (polling interval in milliseconds

  0x07,   // bLength: Endpoint descriptor pwLength
  USB_DESC_TYPE_ENDPOINT,   // bDescriptorType
  WINUSBCOMM_IO32EPOUT_ADDR,   // bEndpointAddress
  USBD_EP_TYPE_BULK,   // bmAttributes
  USB_DESC_W(IO32_MAX_EP_PACKET), // wMaxPacketSize
  0x00     // bInterval (polling interval in milliseconds
};
COMPILETIME_ASSERT(USB_IO32_CONFIG_DESC_SIZ == sizeof(s_abyUSBD_IO32_ConfigurationDescriptor));

static uint8_t s_abyUSBD_IO32_DeviceQualifierDesc[] =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  IO32_MAX_EP_PACKET,
  0x01,
  0x00,
};
COMPILETIME_ASSERT(USB_LEN_DEV_QUALIFIER_DESC == sizeof(s_abyUSBD_IO32_DeviceQualifierDesc));

#ifdef USE_MS_OS_DESC_10
// MS OS String descriptor to tell Windows that it may query for other descriptors
// It's a standard string descriptor.
// Windows will only query for OS descriptors once!
// Delete the information about already queried devices in registry by deleting:
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\usbflags\VVVVPPPPRRRR
static const uint8_t USBD_IO32_MS_OS_StringDescriptor[] =
{
  0x12,           //  bLength           1 0x12  Length of the descriptor
  0x03,           //  bDescriptorType   1 0x03  Descriptor type
                  //  qwSignature      14 ‘MSFT100’ Signature field
  0x4D, 0x00,     //  'M'
  0x53, 0x00,     //  'S'
  0x46, 0x00,     //  'F'
  0x54, 0x00,     //  'T'
  0x31, 0x00,     //  '1'
  0x30, 0x00,     //  '0'
  0x30, 0x00,     //  '0'
  MS_VendorCode,  //  bMS_VendorCode    1 Vendor-specific Vendor code
  0x00            //  bPad              1 0x00  Pad field
};


// This associates winusb driver with the device
static uint8_t USBD_IO32_Extended_Compat_ID_OS_Desc[USB_IO32_COMPAT_ID_OS_DESC_SIZ] =
{
                                                    //    +-- Offset in descriptor
                                                    //    |             +-- Size
                                                    //    v             v
  USB_IO32_COMPAT_ID_OS_DESC_SIZ, 0, 0, 0,          //    0 dwLength    4 DWORD The pwLength, in bytes, of the complete extended compat ID descriptor
  0x00, 0x01,                                       //    4 bcdVersion  2 BCD The descriptor’s version number, in binary coded decimal (BCD) format
  0x04, 0x00,                                       //    6 wIndex      2 WORD  An byIndex that identifies the particular OS feature descriptor
  io32stm32usbinterface_NumInterfaces,              //    8 bCount      1 BYTE  The number of custom property sections
  0, 0, 0, 0, 0, 0, 0,                              //    9 RESERVED    7 BYTEs Reserved
                                                    //    =====================
                                                    //                 16

                                                    //   +-- Offset from function section start
                                                    //   |                        +-- Size
                                                    //   v                        v
  io32stm32usbinterface_WinUSBComm,                 //   0  bFirstInterfaceNumber 1 BYTE  The interface or function number
  0,                                                //   1  RESERVED              1 BYTE  Reserved
  0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00,   //   2  compatibleID          8 BYTEs The function’s compatible ID      ("WINUSB")
  0, 0, 0, 0, 0, 0, 0, 0,                           //  10  subCompatibleID       8 BYTEs The function’s subcompatible ID
  0, 0, 0, 0, 0, 0,                                 //  18  RESERVED              6 BYTEs Reserved
                                                    //  =================================
                                                    //                           24
};

// Properties are added to:
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_xxxx&PID_xxxx\sssssssss\Device Parameters
// Use USBDeview or similar to uninstall

static uint8_t USBD_IO32_Extended_Properties_OS_Desc[0xC8] =
{
  0xC8, 0x00, 0x00, 0x00,   // 0 dwLength   4 DWORD The pwLength, in bytes, of the complete extended properties descriptor
  0x00, 0x01,               // 4 bcdVersion 2 BCD   The descriptor’s version number, in binary coded decimal (BCD) format
  0x05, 0x00,               // 6 wIndex     2 WORD  The byIndex for extended properties OS descriptors
  0x02, 0x00,               // 8 wCount     2 WORD  The number of custom property sections that follow the header section
                            // ====================
                            //             10
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  0x84, 0x00, 0x00, 0x00,   //  0       dwSize                  4 DWORD             The size of this custom properties section
  0x01, 0x00, 0x00, 0x00,   //  4       dwPropertyDataType      4 DWORD             Property data format
  0x28, 0x00,               //  8       wPropertyNameLength     2 DWORD             Property name pwLength
                            // ========================================
                            //                                 10
                            // 10       bPropertyName         PNL WCHAR[]           The property name
  'D',0, 'e',0, 'v',0, 'i',0, 'c',0, 'e',0, 'I',0, 'n',0,
  't',0, 'e',0, 'r',0, 'f',0, 'a',0, 'c',0, 'e',0, 'G',0,
  'U',0, 'I',0, 'D',0, 0,0,
                            // ========================================
                            //                                 40 (0x28)

  0x4E, 0x00, 0x00, 0x00,   // 10 + PNL dwPropertyDataLength    4 DWORD             Length of the buffer holding the property data
                            // ========================================
                            //                                  4
    // 14 + PNL bPropertyData         PDL Format-dependent  Property data
  '{',0, 'E',0, 'A',0, '0',0, 'B',0, 'D',0, '5',0, 'C',0,
  '3',0, '-',0, '5',0, '0',0, 'F',0, '3',0, '-',0, '4',0,
  '8',0, '8',0, '8',0, '-',0, '8',0, '4',0, 'B',0, '4',0,
  '-',0, '7',0, '4',0, 'E',0, '5',0, '0',0, 'E',0, '1',0,
  '6',0, '4',0, '9',0, 'D',0, 'B',0, '}',0,  0 ,0,
                            // ========================================
                            //                                 78 (0x4E)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  0x3A, 0x00, 0x00, 0x00,   //  0 dwSize 0x00000030 (58 bytes)
  0x01, 0x00, 0x00, 0x00,   //  4 dwPropertyDataType 0x00000001 (Unicode string)
  0x0C, 0x00,               //  8 wPropertyNameLength 0x000C (12 bytes)
                            // ========================================
                            //                                  10
  'L',0, 'a',0, 'b',0, 'e',0, 'l',0, 0,0,
                            // 10 bPropertyName “Label”
                            // ========================================
                            //                                  12
  0x20, 0x00, 0x00, 0x00,   // 22 dwPropertyDataLength 0x00000016 (32 bytes)
                            // ========================================
                            //                                  4
  'I',0, 'O',0, '3',0, '2',0, ' ',0, 'U',0, 'S',0, 'B',0, ' ',0, 'W',0, 'g',0, 'g',0, 'l',0, 'e',0, 'r',0,  0,0
                            // 26 bPropertyData “WinUSBComm Device”
                            // ========================================
                            //                                  32

};
// Experiment with these to try to remove device from "Safely Remove Hardware" list:
// HKR,,"SurpriseRemovalOK",0x00010001,1
// HKR,,"Removable",0x00010001,1
// HKR,,"RemovalPolicy",0x00010001,3
#endif

enum EwDescriptorType
{
  MS_OS_20_SET_HEADER_DESCRIPTOR        = 0x00,
  MS_OS_20_SUBSET_HEADER_CONFIGURATION  = 0x01,
  MS_OS_20_SUBSET_HEADER_FUNCTION       = 0x02,
  MS_OS_20_FEATURE_COMPATBLE_ID         = 0x03,
  MS_OS_20_FEATURE_REG_PROPERTY         = 0x04,
  MS_OS_20_FEATURE_MIN_RESUME_TIME      = 0x05,
  MS_OS_20_FEATURE_MODEL_ID             = 0x06,
  MS_OS_20_FEATURE_CCGP_DEVICE          = 0x07
};


#if 0

#define MS_OS_20_DESC_SET_HEADER_SIZE 10
#define MS_OS_20_DESC_CCGP_SIZE 4
#define MS_OS_20_DESC_CONF_HEADER_SIZE 8
#define MS_OS_20_DESC_FUN_HEADER_SIZE 8
#define MS_OS_20_DESC_COMAPTIBLE_ID_FEATURE_SIZE 20

#define IO32_CONFIG0_FUN0_REG0_SIZE (10 + 40 + 78)
#define IO32_CONFIG0_FUN0_REG1_SIZE (10 + 12 + 34)
#define IO32_CONFIG0_FUN0_SUBSET_SIZE (MS_OS_20_DESC_FUN_HEADER_SIZE + MS_OS_20_DESC_COMAPTIBLE_ID_FEATURE_SIZE + IO32_CONFIG0_FUN0_REG0_SIZE + IO32_CONFIG0_FUN0_REG1_SIZE)

#define IO32_CONFIG0_SUBSET_SIZE (MS_OS_20_DESC_CONF_HEADER_SIZE + IO32_CONFIG0_FUN0_SUBSET_SIZE)

#define IO32_MS_OS_20_DESCRIPTOR_SIZE (MS_OS_20_DESC_SET_HEADER_SIZE + MS_OS_20_DESC_CCGP_SIZE + IO32_CONFIG0_SUBSET_SIZE)

static const unsigned char sc_abyIO32_MSOS20Descriptor[] =
{
  // Microsoft OS 2.0 Descriptor Set Header
  MS_OS_20_DESC_SET_HEADER_SIZE, 0x00,  // wLength - The pwLength, in bytes, of this header. Shall be set to 10.
  MS_OS_20_SET_HEADER_DESCRIPTOR, 0x00, // wDescriptorType - Of EwDescriptorType
  0x00, 0x00, 0x03, 0x06,               // dwWindowsVersion - 0x06030000 for Windows Blue
  IO32_MS_OS_20_DESCRIPTOR_SIZE, 0x00,  // wTotalLength - The size of entire MS OS 2.0 descriptor set. The value shall match the value in the descriptor set information structure.
//10
    // Microsoft OS 2.0 CCGP device descriptor
    MS_OS_20_DESC_CCGP_SIZE, 0x00,        // wLength - The pwLength, in bytes, of this descriptor. Shall be set to 4.
    MS_OS_20_FEATURE_CCGP_DEVICE, 0x00,   // wDescriptorType - Of EwDescriptorType
//4
    // Microsoft OS 2.0 configuration subset header
    MS_OS_20_DESC_CONF_HEADER_SIZE, 0x00,              // wLength - The pwLength, in bytes, of this subset header. Shall be set to 8.
    MS_OS_20_SUBSET_HEADER_CONFIGURATION, 0x00,   // wDescriptorType - Of EwDescriptorType
    0,                                            // bConfigurationValue - The configuration value for the USB configuration to which this subset applies.
    0x00,                                         // bReserved
    IO32_CONFIG0_SUBSET_SIZE, 0x00,               // wTotalLength - The size of entire configuration subset including this header.
//8
      // Microsoft OS 2.0 function subset header
      MS_OS_20_DESC_FUN_HEADER_SIZE, 0x00,          // wLength - The pwLength, in bytes, of this subset header. Shall be set to 8.
      MS_OS_20_SUBSET_HEADER_FUNCTION, 0x00,        // wDescriptorType - Of EwDescriptorType
      0,                                            // bFirstInterface - The interface number for the first interface of the function to which this subset applies.
      0x00,                                         // bReserved
      IO32_CONFIG0_FUN0_SUBSET_SIZE, 0x00,          // wSubsetLength - The size of entire function subset including this header.
//8
      // Microsoft OS 2.0 Compatible ID feature descriptor
        MS_OS_20_DESC_COMAPTIBLE_ID_FEATURE_SIZE, 0x00,  // wLength - The pwLength, bytes, of the compatible ID descriptor including value descriptors. Shall be set to 20.
        MS_OS_20_FEATURE_COMPATBLE_ID, 0x00,             // wDescriptorType - Of EwDescriptorType
        'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,        // CompatibleID - Compatible ID String
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // SubCompatibleID - Sub-compatible ID String
//20
        // Microsoft OS 2.0 registry property feature descriptor
        IO32_CONFIG0_FUN0_REG0_SIZE, 0x00,                    // wLength - The pwLength, in bytes, of this descriptor.
        MS_OS_20_FEATURE_REG_PROPERTY, 0x00,                  // wDescriptorType - Of EwDescriptorType
        0x01, 0x00,                                           // wPropertyDataType - The type of registry property. A NULL-terminated Unicode String (REG_SZ)
        40, 0x00,                                             // wPropertyNameLength - The pwLength of the property name.
        // PropertyName - The name of registry property.
        'D',0, 'e',0, 'v',0, 'i',0, 'c',0, 'e',0, 'I',0, 'n',0, 't',0, 'e',0, 'r',0, 'f',0, 'a',0, 'c',0, 'e',0, 'G',0, 'U',0, 'I',0, 'D',0, 0,0,
        78, 0x00,                                           // wPropertyDataLength - The pwLength of property data.
        // PropertyData - Property data
        '{',0, 'E',0, 'A',0, '0',0, 'B',0, 'D',0, '5',0, 'C',0,
        '3',0, '-',0, '5',0, '0',0, 'F',0, '3',0, '-',0, '4',0,
        '8',0, '8',0, '8',0, '-',0, '8',0, '4',0, 'B',0, '4',0,
        '-',0, '7',0, '4',0, 'E',0, '5',0, '0',0, 'E',0, '1',0,
        '6',0, '4',0, '9',0, 'D',0, 'B',0, '}',0,  0 ,0,
//10+40+78
        // Microsoft OS 2.0 registry property feature descriptor
        IO32_CONFIG0_FUN0_REG1_SIZE, 0x00,                    // wLength - The pwLength, in bytes, of this descriptor.
        MS_OS_20_FEATURE_REG_PROPERTY, 0x00,                  // wDescriptorType - Of EwDescriptorType
        0x01, 0x00,                                           // wPropertyDataType - The type of registry property. A NULL-terminated Unicode String (REG_SZ)
        12, 0x00,                                             // wPropertyNameLength - The pwLength of the property name.
        // PropertyName - The name of registry property.
        'L',0, 'a',0, 'b',0, 'e',0, 'l',0, 0,0,
        34, 0x00,                                             // wPropertyDataLength - The pwLength of property data.
        // PropertyData - Property data
        'I',0, 'O',0, '3',0, '2',0, ' ',0, 'U',0, 'S',0, 'B',0, ' ',0, 'W',0, 'i',0, 'g',0, 'g',0, 'l',0, 'e',0, 'r',0, 0,0
//10+12+34
};
#else
#define IO32_MS_OS_20_DESCRIPTOR_SIZE (30 + 10+40+78 + 10+12+34)
static const unsigned char sc_abyIO32_MSOS20Descriptor[] =
{
// Microsoft OS 2.0 descriptor set header
0x0A, 0x00,  // Descriptor size (10)
0x00, 0x00,  // MS OS 2.0 descriptor set header
0x00, 0x00, 0x03, 0x06,  // Windows version (8.1) (0x06030000)
IO32_MS_OS_20_DESCRIPTOR_SIZE, 0x00,  // Size, MS OS 2.0 descriptor set

// Microsoft OS 2.0 compatible ID descriptor
0x14, 0x00,  // wLength
0x03, 0x00,  // MS_OS_20_FEATURE_COMPATIBLE_ID
'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

// Microsoft OS 2.0 registry property feature descriptor
10+40+78, 0x00,                                       // wLength - The pwLength, in bytes, of this descriptor.
MS_OS_20_FEATURE_REG_PROPERTY, 0x00,                  // wDescriptorType - Of EwDescriptorType
0x01, 0x00,                                           // wPropertyDataType - The type of registry property. A NULL-terminated Unicode String (REG_SZ)
40, 0x00,                                             // wPropertyNameLength - The pwLength of the property name.
// PropertyName - The name of registry property.
'D',0, 'e',0, 'v',0, 'i',0, 'c',0, 'e',0, 'I',0, 'n',0, 't',0, 'e',0, 'r',0, 'f',0, 'a',0, 'c',0, 'e',0, 'G',0, 'U',0, 'I',0, 'D',0, 0,0,
78, 0x00,                                           // wPropertyDataLength - The pwLength of property data.
// PropertyData - Property data
'{',0, 'E',0, 'A',0, '0',0, 'B',0, 'D',0, '5',0, 'C',0,
'3',0, '-',0, '5',0, '0',0, 'F',0, '3',0, '-',0, '4',0,
'8',0, '8',0, '8',0, '-',0, '8',0, '4',0, 'B',0, '4',0,
'-',0, '7',0, '4',0, 'E',0, '5',0, '0',0, 'E',0, '1',0,
'6',0, '4',0, '9',0, 'D',0, 'B',0, '}',0,  0 ,0,

// Microsoft OS 2.0 registry property feature descriptor
10+12+34, 0x00,                                       // wLength - The pwLength, in bytes, of this descriptor.
MS_OS_20_FEATURE_REG_PROPERTY, 0x00,                  // wDescriptorType - Of EwDescriptorType
0x01, 0x00,                                           // wPropertyDataType - The type of registry property. A NULL-terminated Unicode String (REG_SZ)
12, 0x00,                                             // wPropertyNameLength - The pwLength of the property name.
// PropertyName - The name of registry property.
'L',0, 'a',0, 'b',0, 'e',0, 'l',0, 0,0,
34, 0x00,                                             // wPropertyDataLength - The pwLength of property data.
// PropertyData - Property data
'I',0, 'O',0, '3',0, '2',0, ' ',0, 'U',0, 'S',0, 'B',0, ' ',0, 'W',0, 'i',0, 'g',0, 'g',0, 'l',0, 'e',0, 'r',0, 0,0


};
#endif
COMPILETIME_ASSERT(IO32_MS_OS_20_DESCRIPTOR_SIZE == sizeof(sc_abyIO32_MSOS20Descriptor));

#define IO32_WEB_USB_DESCRIPTOR_SIZE 38
static const char sc_abyIO32_WebUSBDescriptor[] =
{
  IO32_WEB_USB_DESCRIPTOR_SIZE, // bLength - Size of this descriptor.
  3,                            // bDescriptorType - WEBUSB_URL
  255,                          // bScheme - entire URL, including scheme, is encoded in the URL field
  'h','t','t','p',':','/','/','s','e','a','r','c','h','i','n','g','f','o','r','b','i','t','.','b','l','o','g','s','p','o','t','.','s','i',0
};
COMPILETIME_ASSERT(IO32_WEB_USB_DESCRIPTOR_SIZE == sizeof(sc_abyIO32_WebUSBDescriptor));

#define IO32_USB_BOS_DESCRIPTOR_SIZE (5 + 28 + 24)
static const uint8_t sc_abyUSBD_IO32_BOS[] =
{
  // BOS descriptor
  0x05,         // Descriptor size
  0x0F,         // Descriptor type (BOS)
  IO32_USB_BOS_DESCRIPTOR_SIZE, 0x00,     // Length of this + subordinate descriptors
  2,            // Number of subordinate descriptors

    // Microsoft OS 2.0 Platform Capability Descriptor Header
    0x1C, // bLength - 28 bytes
    0x10, // bDescriptorType - 16
    0x05, // bDevCapability – 5 for Platform Capability
    0x00, // bReserved - 0
    0xDF, 0x60, 0xDD, 0xD8, // MS_OS_20_Platform_Capability_ID -
    0x89, 0x45, 0xC7, 0x4C, // {D8DD60DF-4589-4CC7-9CD2-659D9E648A9F}
    0x9C, 0xD2, 0x65, 0x9D, //
    0x9E, 0x64, 0x8A, 0x9F, //
    // Descriptor Information Set for Windows 8.1 or later
    0x00, 0x00, 0x03, 0x06, // dwWindowsVersion – 0x06030000 for Windows Blue
    sizeof(sc_abyIO32_MSOS20Descriptor), 0x00, // wLength – size of MS OS 2.0 descriptor set
    MS_VendorCode2, // bMS_VendorCode
    0x00, // bAltEnumCmd – 0 Does not support alternate enum


    // WebUSB Platform Capability descriptor
    24,                 // Descriptor size
    0x10,               // Descriptor type (Device Capability)
    0x05,               // Capability type (Platform)
    0x00,               // Reserved
                        // WebUSB Platform Capability ID (3408b638-09a9-47a0-8bfd-a0768815b665)
    0x38, 0xB6, 0x08, 0x34,   0xA9, 0x09,   0xA0, 0x47,   0x8B, 0xFD,   0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65,
    0x00, 0x01,         // WebUSB version 1.0
    WU_VendorCode,      // Vendor-assigned WebUSB request code
    0x1                 // Landing page
};
COMPILETIME_ASSERT(IO32_USB_BOS_DESCRIPTOR_SIZE == sizeof(sc_abyUSBD_IO32_BOS));

uint8_t *s_BOSDescriptor(USBD_SpeedTypeDef eSpeed, uint16_t * pwLength)
{
  if ( pwLength )
  {
    *pwLength = sizeof(sc_abyUSBD_IO32_BOS);
  }
  return (uint8_t *)sc_abyUSBD_IO32_BOS;
}

static uint8_t USBD_IO32_Init(USBD_HandleTypeDef *pDev, uint8_t byConfigurationIndex)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  USBD_LL_OpenEP(pDev, WINUSBCOMM_IO32EPOUT_ADDR, USBD_EP_TYPE_BULK, IO32_MAX_EP_PACKET);
  USBD_LL_OpenEP(pDev, WINUSBCOMM_IO32EPIN_ADDR , USBD_EP_TYPE_BULK, IO32_MAX_EP_PACKET);
  return USBD_OK;
}

static uint8_t USBD_IO32_DeInit(USBD_HandleTypeDef *pDev, uint8_t byConfigurationIndex)
{
  USBD_LL_CloseEP(pDev, WINUSBCOMM_IO32EPOUT_ADDR);
  USBD_LL_CloseEP(pDev, WINUSBCOMM_IO32EPIN_ADDR);

  pDev->pClassData  = NULL;

  return USBD_OK;
}

static uint8_t USBD_IO32_SetupStandrad(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;

  switch ( pSetupRequest->bRequest )
  {
  case USB_REQ_GET_DESCRIPTOR :
    USBD_CtlError(pDev, pSetupRequest);
    return USBD_FAIL;
  case USB_REQ_GET_INTERFACE :
    USBD_CtlSendData (pDev, (uint8_t *)&psSTM32USB->m_nCurrentInterface, 1);
    break;

  case USB_REQ_SET_INTERFACE :
    psSTM32USB->m_nCurrentInterface = (uint8_t)(pSetupRequest->wValue);
    break;

  case USB_REQ_CLEAR_FEATURE:
    USBD_LL_FlushEP(pDev, (uint8_t)pSetupRequest->wIndex);
    USBD_LL_CloseEP (pDev, (uint8_t)pSetupRequest->wIndex);
    if((((uint8_t)pSetupRequest->wIndex) & 0x80) == 0x80)
    {
      USBD_LL_OpenEP(pDev, WINUSBCOMM_IO32EPIN_ADDR, USBD_EP_TYPE_BULK, IO32_MAX_EP_PACKET);
    }
    else
    {
      USBD_LL_OpenEP(pDev, WINUSBCOMM_IO32EPOUT_ADDR, USBD_EP_TYPE_BULK, IO32_MAX_EP_PACKET);
    }
    break;
  default:
    USBD_CtlError(pDev, pSetupRequest);
    return USBD_FAIL;
  }
  return USBD_OK;
}
#ifdef USE_MS_OS_DESC_10
static uint8_t USBD_IO32_GetMSExtendedCompatIDOSDescriptor(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->wIndex )
  {
  case 0x04:
    USBD_CtlSendData (pDev, USBD_IO32_Extended_Compat_ID_OS_Desc, pSetupRequest->wLength);
    break;
  default:
   USBD_CtlError(pDev, pSetupRequest);
   return USBD_FAIL;
  }
  return USBD_OK;
}
static uint8_t USBD_IO32_GetMSExtendedPropertiesOSDescriptor(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  uint8_t byInterfaceIndex = (uint8_t)pSetupRequest->wValue;
  if ( pSetupRequest->wIndex != 0x05 )
  {
    USBD_CtlError(pDev, pSetupRequest);
    return USBD_FAIL;
  }
  switch ( byInterfaceIndex )
  {
  case io32stm32usbinterface_WinUSBComm:
    USBD_CtlSendData(pDev, USBD_IO32_Extended_Properties_OS_Desc, pSetupRequest->wLength);
    break;
  default:
    USBD_CtlError(pDev, pSetupRequest);
    return USBD_FAIL;
  }
  return USBD_OK;
}
#else
static uint8_t USBD_IO32_GetMSOSDescriptor20(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->wIndex )
  {
  case 0x07:
    USBD_CtlSendData(pDev, (unsigned char *)sc_abyIO32_MSOS20Descriptor, pSetupRequest->wLength);
    break;
  default:
   USBD_CtlError(pDev, pSetupRequest);
   return USBD_FAIL;
  }
  return USBD_OK;
}
#endif
static uint8_t USBD_IO32_GetWebUSBDescriptor(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->wIndex )
  {
  case 0x02:
    USBD_CtlSendData(pDev, (unsigned char *)sc_abyIO32_WebUSBDescriptor, pSetupRequest->wLength);
    break;
  default:
   USBD_CtlError(pDev, pSetupRequest);
   return USBD_FAIL;
  }
  return USBD_OK;
}

static uint8_t USBD_IO32_SetupVendorDevice(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->bRequest )
  {
#ifdef USE_MS_OS_DESC_10
  case MS_VendorCode: return USBD_IO32_GetMSExtendedCompatIDOSDescriptor(pDev, pSetupRequest);
#else
  case MS_VendorCode2: return USBD_IO32_GetMSOSDescriptor20(pDev, pSetupRequest);
#endif
  case WU_VendorCode: return USBD_IO32_GetWebUSBDescriptor(pDev, pSetupRequest);
  default:
    break;
  }
  USBD_CtlError(pDev, pSetupRequest);
  return USBD_FAIL;
}

static uint8_t usbd_io32_processResponse(USBD_HandleTypeDef *pDev, const SWinUSBSetupResponse *pResponse)
{
  switch ( pResponse->m_eResponse )
  {
  case ewinusbsetupresponseOK: break;
  case ewinusbsetupresponseSendToHostOnEP0:       USBD_CtlSendData(pDev, pResponse->m_pbyData, pResponse->m_dwByteCount); break;
  case ewinusbsetupresponseReceiveFromHostOnEP0:  USBD_CtlPrepareRx(pDev, pResponse->m_pbyData, pResponse->m_dwByteCount); break;
  case ewinusbsetupresponseSendToHostOnEPx:       USBD_LL_Transmit(pDev, WINUSBCOMM_IO32EPIN_ADDR, pResponse->m_pbyData, pResponse->m_dwByteCount); break;
  case ewinusbsetupresponseReceiveFromHostOnEPy:  USBD_LL_PrepareReceive(pDev, WINUSBCOMM_IO32EPOUT_ADDR, pResponse->m_pbyData, pResponse->m_dwByteCount); break;
  default: return USBD_FAIL;
  }
  return USBD_OK;
}

static uint8_t USBD_IO32_SetupVendorInterface(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  SWinUSBSetupResponse setupResponse = { 0 };
  switch ( pSetupRequest->wIndex )
  {
  case io32stm32usbinterface_WinUSBComm:
    setupResponse = WinUSBComm_SetupVendorInterface(psSTM32USB->m_pWinUSBContext, pSetupRequest->bRequest);
    if ( USBD_OK != usbd_io32_processResponse(pDev, &setupResponse) )
    {
      USBD_CtlError(pDev, pSetupRequest);
      return USBD_FAIL;
    }
    break;
  default:
    USBD_CtlError(pDev, pSetupRequest);
    return USBD_FAIL;
  }
  return USBD_OK;
}
static uint8_t USBD_IO32_SetupVendor(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->bmRequest & USB_REQ_RECIPIENT_MASK )
  {
  case USB_REQ_RECIPIENT_DEVICE:
    return USBD_IO32_SetupVendorDevice(pDev, pSetupRequest);
  case USB_REQ_RECIPIENT_INTERFACE:
    return
#ifdef USE_MS_OS_DESC_10
        ( MS_VendorCode == pSetupRequest->bRequest ) ? USBD_IO32_GetMSExtendedPropertiesOSDescriptor(pDev, pSetupRequest) :
#endif
            USBD_IO32_SetupVendorInterface(pDev, pSetupRequest);
  case USB_REQ_RECIPIENT_ENDPOINT:
    // fall through
  default:
    break;
  }
  USBD_CtlError(pDev, pSetupRequest);
  return USBD_FAIL;
}
static uint8_t USBD_IO32_Setup(USBD_HandleTypeDef *pDev, USBD_SetupReqTypedef *pSetupRequest)
{
  switch ( pSetupRequest->bmRequest & USB_REQ_TYPE_MASK )
  {
  case USB_REQ_TYPE_CLASS:
    break;
  case USB_REQ_TYPE_STANDARD:
    return USBD_IO32_SetupStandrad(pDev, pSetupRequest);
  case USB_REQ_TYPE_VENDOR:
    return USBD_IO32_SetupVendor(pDev, pSetupRequest);
  default:
    break;
  }
  USBD_CtlError(pDev, pSetupRequest);
  return USBD_FAIL;
}

static uint8_t *USBD_IO32_GetCfgDesc(uint16_t *pwLength)
{
  *pwLength = sizeof(s_abyUSBD_IO32_ConfigurationDescriptor);
  return s_abyUSBD_IO32_ConfigurationDescriptor;
}

uint8_t *USBD_IO32_DeviceQualifierDescriptor(uint16_t *pwLength)
{
  *pwLength = sizeof(s_abyUSBD_IO32_DeviceQualifierDesc);
  return s_abyUSBD_IO32_DeviceQualifierDesc;
}

static uint8_t USBD_IO32_DataIn(USBD_HandleTypeDef *pDev, uint8_t byEP)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  switch ( byEP & 0x7F )
  {
  case WINUSBCOMM_IO32EPIN_ADDR & 0x7F: WinUSBComm_DataIn(psSTM32USB->m_pWinUSBContext); break;
  default: break;
  }
  return USBD_OK;
}

static uint8_t USBD_IO32_EP0_RxReady(USBD_HandleTypeDef *pDev)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  USBD_SetupReqTypedef *pSetupRequest = &pDev->request;

  SWinUSBSetupResponse response = { 0 };

  if ( (pSetupRequest->bmRequest & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) == (USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_INTERFACE) )
  {
    switch ( pSetupRequest->wIndex )
    {
    case io32stm32usbinterface_WinUSBComm:
      response = WinUSBComm_EP0_RxReady(psSTM32USB->m_pWinUSBContext, pSetupRequest->bRequest);
      usbd_io32_processResponse(pDev, &response);
      break;
    default:
      break;
    }
  }

  return USBD_OK;
}

static uint8_t USBD_IO32_EP0_TxReady(USBD_HandleTypeDef *pDev)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  USBD_SetupReqTypedef *pSetupRequest = &pDev->request;

  SWinUSBSetupResponse response = { 0 };

  if ( (pSetupRequest->bmRequest & (USB_REQ_TYPE_MASK | USB_REQ_RECIPIENT_MASK)) == (USB_REQ_TYPE_VENDOR | USB_REQ_RECIPIENT_INTERFACE) )
  {
    switch ( pSetupRequest->wIndex )
    {
    case io32stm32usbinterface_WinUSBComm:
      response = WinUSBComm_EP0_TxReady(psSTM32USB->m_pWinUSBContext, pSetupRequest->bRequest);
      usbd_io32_processResponse(pDev, &response);
      break;
    default:
      break;
    }
  }

  return USBD_OK;
}

static uint8_t USBD_IO32_SOF(USBD_HandleTypeDef *pDev)
{
  return USBD_OK;
}
static uint8_t USBD_IO32_IsoINIncomplete(USBD_HandleTypeDef *pDev, uint8_t byEP)
{
  return USBD_OK;
}
static uint8_t USBD_IO32_IsoOutIncomplete(USBD_HandleTypeDef *pDev, uint8_t byEP)
{
  return USBD_OK;
}
static uint8_t USBD_IO32_DataOut(USBD_HandleTypeDef *pDev, uint8_t byEP)
{
  SIO32STM32USB *psSTM32USB = (SIO32STM32USB *)pDev->pUserData;
  switch ( byEP & 0x7F )
  {
    case WINUSBCOMM_IO32EPOUT_ADDR & 0x7F: WinUSBComm_DataOut(psSTM32USB->m_pWinUSBContext); break;
    default: break;
  }
  return USBD_OK;
}
static uint8_t *USBD_IO32_GetDeviceQualifierDesc(uint16_t *pwLength)
{
  *pwLength = sizeof(s_abyUSBD_IO32_DeviceQualifierDesc);
  return s_abyUSBD_IO32_DeviceQualifierDesc;
}

#if ( USBD_SUPPORT_USER_STRING == 1 )
static uint8_t * USBD_IO32_GetUsrStrDescriptor(struct _USBD_HandleTypeDef *pDev, uint8_t byIndex,  uint16_t *pwLength)
{
  *pwLength = 0;
#if USE_MS_OS_DESC_10
  if ( 0xEE == byIndex )
  {
    *pwLength = sizeof(USBD_IO32_MS_OS_StringDescriptor);
    return USBD_IO32_MS_OS_StringDescriptor;
  }
#endif
  return NULL;
}
#endif

USBD_ClassTypeDef  USBD_IO32_ClassDriver =
{
  USBD_IO32_Init,
  USBD_IO32_DeInit,
  USBD_IO32_Setup,
  USBD_IO32_EP0_TxReady,
  USBD_IO32_EP0_RxReady,
  USBD_IO32_DataIn,
  USBD_IO32_DataOut,
  USBD_IO32_SOF,
  USBD_IO32_IsoINIncomplete,
  USBD_IO32_IsoOutIncomplete,
  USBD_IO32_GetCfgDesc,
  USBD_IO32_GetCfgDesc,
  USBD_IO32_GetCfgDesc,
  USBD_IO32_GetDeviceQualifierDesc,
#if ( USBD_SUPPORT_USER_STRING == 1 )
  USBD_IO32_GetUsrStrDescriptor
#endif
};

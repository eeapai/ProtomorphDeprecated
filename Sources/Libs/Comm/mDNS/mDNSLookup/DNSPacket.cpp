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
#include "DNSPacket.h"
#include <iostream>

#ifndef BE
#define LSB2WORD(buff) *((uint16_t *)(buff))
#define LSB2DWORD(buff) *((uint32_t *)(buff))
#define MSB2WORD(buff) ((((uint16_t)(*(buff))) << 8) | (*((buff) + 1)))
#define MSB2DWORD(buff) ((((uint32_t)(MSB2WORD(buff))) << 16) | (MSB2WORD((buff) + 2)))
#endif

static uint16_t s_ReadDwordMSB(const uint8_t *pbyData, uint32_t &rdwOffset)
{
  uint32_t dwData = MSB2DWORD(pbyData + rdwOffset);
  rdwOffset += 4;
  return dwData;
}
static uint16_t s_ReadWordLSB(const uint8_t *pbyData, uint32_t &rdwOffset)
{
  uint16_t wData = LSB2WORD(pbyData + rdwOffset);
  rdwOffset += 2;
  return wData;
}
static uint16_t s_ReadWordMSB(const uint8_t *pbyData, uint32_t &rdwOffset)
{
  uint16_t wData = MSB2WORD(pbyData + rdwOffset);
  rdwOffset += 2;
  return wData;
}
static uint8_t s_ReadByte(const uint8_t *pbyData, uint32_t &rdwOffset)
{
  return pbyData[rdwOffset++];
}
static char s_ReadChar(const uint8_t *pbyData, uint32_t &rdwOffset)
{
  return (char)(pbyData[rdwOffset++]);
}


static const uint16_t sc_wQRMask        = 0x8000;
static const uint16_t sc_wOpCodeMask    = 0x7800;
static const uint16_t sc_wOpCodeMaskLSb = 11;
static const uint16_t sc_wAAMask        = 0x0400;
static const uint16_t sc_wTCMask        = 0x0200;
static const uint16_t sc_wRDMask        = 0x0100;
static const uint16_t sc_wRAMask        = 0x0080;
static const uint16_t sc_wRCODEMask     = 0x000F;
static const uint16_t sc_wRCODELSb      = 0;

// http://www.tcpipguide.com/free/t_DNSMessageHeaderandQuestionSectionFormat.htm

uint32_t ParseDNSHeader(const uint8_t *pbyData, uint32_t dwAvailableByteCount, SDNSHeader *pPacket)
{
  uint32_t dwUsed = 0;
  pPacket->m_wID = s_ReadWordMSB(pbyData, dwUsed);
  uint16_t wFlags = s_ReadWordMSB(pbyData, dwUsed);
  pPacket->m_byOpCode = (wFlags & sc_wOpCodeMask) >> sc_wOpCodeMaskLSb;
  pPacket->m_bQR = 0 != (wFlags & sc_wQRMask);
  pPacket->m_bAA = 0 != (wFlags & sc_wAAMask);
  pPacket->m_bTC = 0 != (wFlags & sc_wTCMask);
  pPacket->m_bRD = 0 != (wFlags & sc_wRDMask);
  pPacket->m_bRA = 0 != (wFlags & sc_wRAMask);
  pPacket->m_byRCODE = (wFlags & sc_wRCODEMask) >> sc_wRCODELSb;

  pPacket->m_wQDCOUNT = s_ReadWordMSB(pbyData, dwUsed);
  pPacket->m_wANCOUNT = s_ReadWordMSB(pbyData, dwUsed);
  pPacket->m_wNSCOUNT = s_ReadWordMSB(pbyData, dwUsed);
  pPacket->m_wARCOUNT = s_ReadWordMSB(pbyData, dwUsed);

  return dwUsed;
}

static const uint8_t sc_byPointerMask = 0xC0;

uint32_t ParseDNSName(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, std::string &rstrName)
{
  uint32_t dwUsed = dwOffset;
  
  uint8_t byLength = s_ReadByte(pbyData, dwUsed);
  if ( !byLength )
  {
    return dwUsed;
  }

  bool bIsPointer = sc_byPointerMask == (byLength & sc_byPointerMask);
  if ( bIsPointer )
  {
    uint16_t wOffset = byLength & ~sc_byPointerMask;
    wOffset <<= 8;
    wOffset |= s_ReadByte(pbyData, dwUsed);
    ParseDNSName(pbyData, wOffset, dwAvailableByteCount, rstrName);
    return dwUsed;
  }

  if ( rstrName.length() )
  {
    rstrName.append(".");
  }

  rstrName.append((const char *)pbyData + dwUsed, byLength);
  dwUsed += byLength;
  dwUsed = ParseDNSName(pbyData, dwUsed, dwAvailableByteCount, rstrName);
  return dwUsed;
}

static const uint16_t sc_wClassMSb = 0x8000;

uint32_t ParseDNSQuestion(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, SDNSQuestion *pQuestion)
{
  uint32_t dwUsed = ParseDNSName(pbyData, dwOffset, dwAvailableByteCount, pQuestion->m_strQuestion);
  pQuestion->m_wQTYPE = s_ReadWordMSB(pbyData, dwUsed);
  pQuestion->m_wQCLASS = s_ReadWordMSB(pbyData, dwUsed);
  pQuestion->m_bPreferUnicastResponse = 0 != (pQuestion->m_wQCLASS & sc_wClassMSb);
  pQuestion->m_wQCLASS &= ~sc_wClassMSb;
  return dwUsed;
}

uint32_t ParseDNSDNSCommonResourceRecord(const uint8_t * pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, SDNSCommonResourceRecord * pCRR)
{
  uint32_t dwUsed = ParseDNSName(pbyData, dwOffset, dwAvailableByteCount, pCRR->m_strName);
  pCRR->m_wType = s_ReadWordMSB(pbyData, dwUsed);
  pCRR->m_wClass = s_ReadWordMSB(pbyData, dwUsed);
  pCRR->m_bMemberOfUniqueRRSet = 0 != (pCRR->m_wClass & sc_wClassMSb);
  pCRR->m_wClass &= ~sc_wClassMSb;
  pCRR->m_dwTTL= s_ReadDwordMSB(pbyData, dwUsed);
  pCRR->m_wResourceDataLength = s_ReadWordMSB(pbyData, dwUsed);
  return dwUsed;
}

uint32_t ParseIPv4(const uint8_t * pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, uint8_t * pbyIP)
{
  uint32_t dwUsed = dwOffset;

  pbyIP[0] = s_ReadByte(pbyData, dwUsed);
  pbyIP[1] = s_ReadByte(pbyData, dwUsed);
  pbyIP[2] = s_ReadByte(pbyData, dwUsed);
  pbyIP[3] = s_ReadByte(pbyData, dwUsed);
  return dwUsed;
}


void DNSDumpPacket(const uint8_t *pbyData, uint32_t dwPacketSize, const char *pcszNameFilter /*= nullptr*/)
{
  SDNSHeader DNSHeader;
  uint32_t dwUsed = ParseDNSHeader(pbyData, dwPacketSize, &DNSHeader);
  if ( !pcszNameFilter )
  {
    std::cout << "mDNS ";
    if ( DNSHeader.m_bQR )
    {
      std::cout << "response";
    }
    else
    {
      std::cout << "query";
    }
    std::cout << std::endl;
  }
  for ( uint32_t Q = 0; Q < DNSHeader.m_wQDCOUNT; ++Q )
  {
    SDNSQuestion q;
    dwUsed = ParseDNSQuestion(pbyData, dwUsed, dwPacketSize, &q);
    if ( !pcszNameFilter )
    {
      printf("Type: %d, Class: %d%s ", q.m_wQTYPE, q.m_wQCLASS, q.m_bPreferUnicastResponse ? " prefer unicast" : "");
      std::cout << "? " << q.m_strQuestion << std::endl;
    }
  }

  for ( uint32_t A = 0; A < DNSHeader.m_wANCOUNT; ++A )
  {
    SDNSCommonResourceRecord r;
    dwUsed = ParseDNSDNSCommonResourceRecord(pbyData, dwUsed, dwPacketSize, &r);
    bool bDumpName = true;
    if ( pcszNameFilter )
    {
      bDumpName = !r.m_strName.compare(pcszNameFilter);
    }
    if ( bDumpName )
    {
      std::cout << r.m_strName << " : ";
    }
    if ( (1 == r.m_wType) && (1 == r.m_wClass) )
    {
      uint8_t abyIP[4];
      dwUsed = ParseIPv4(pbyData, dwUsed, dwPacketSize, abyIP);
      if ( bDumpName )
      {
        printf("%d.%d.%d.%d", abyIP[0], abyIP[1], abyIP[2], abyIP[3]);
        std::cout << std::endl;
      }
    }
    else if ( (12 == r.m_wType) && (1 == r.m_wClass) )
    {
      std::string strName;
      dwUsed = ParseDNSName(pbyData, dwUsed, dwPacketSize, strName);
      if ( bDumpName )
      {
        std::cout << strName << std::endl;
      }
    }
    else
    {
      if ( bDumpName )
      {
        printf("Type: %d, Class: %d ", r.m_wType, r.m_wClass);
        std::cout << "unsupported" << std::endl;
      }
      dwUsed += r.m_wResourceDataLength;
    }
  }
}

static const char s_achmDNSQuery_Start[] =
{
  0x00, 0x00, //  Transaction ID
  0x00, 0x00, //  Flags
  0x00, 0x01, //  Number of questions
  0x00, 0x00, //  Number of answers
  0x00, 0x00, //  Number of authority resource records
  0x00, 0x00, //  Number of additional resource records
};

static const char s_achmDNSQuery_End[] =
{
  0x00,       //  Terminator
  0x00, 0x01, //  Type(A record)
  0x00, 0x01, //  Class
};

#define MAX_LABEL_LENGTH (0xC - 1)
size_t DNSMakeQueryPacket(const char *pcszName, uint8_t *pbyDestination, uint32_t dwMaxLen)
{
  size_t sUsed = 0;
  if ( (sUsed + sizeof(s_achmDNSQuery_Start)) <= dwMaxLen )
  {
    memmove(pbyDestination + sUsed, s_achmDNSQuery_Start, sizeof(s_achmDNSQuery_Start));
  }
  sUsed += sizeof(s_achmDNSQuery_Start);

  const char *pcszLabelStart = pcszName;
  const char *pcszLabelEnd = nullptr;
  while ( pcszLabelStart && *pcszLabelStart )
  {
    size_t sLabelLength = 0;
    pcszLabelEnd = strchr(pcszLabelStart, '.');

    if ( pcszLabelEnd )
    {
      sLabelLength = pcszLabelEnd - pcszLabelStart;
      pcszLabelEnd++;
    }
    else
    {
      sLabelLength = strlen(pcszLabelStart);
    }

    if ( sLabelLength > MAX_LABEL_LENGTH )
    {
      return 0;
    }

    if ( (sUsed + 1) <= dwMaxLen )
    {
      pbyDestination[sUsed] = (uint8_t)sLabelLength;
    }
    sUsed++;

    if ( (sUsed + sLabelLength) <= dwMaxLen )
    {
      memmove(pbyDestination + sUsed, pcszLabelStart, sLabelLength);
    }
    sUsed += sLabelLength;

    pcszLabelStart = pcszLabelEnd;
  }

  if ( (sUsed + sizeof(s_achmDNSQuery_End)) <= dwMaxLen )
  {
    memmove(pbyDestination + sUsed, s_achmDNSQuery_End, sizeof(s_achmDNSQuery_End));
  }
  sUsed += sizeof(s_achmDNSQuery_End);
  return sUsed;
}

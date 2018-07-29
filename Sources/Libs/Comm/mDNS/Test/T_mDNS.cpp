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
#include "gtest/gtest.h"

#include "DNSPacket.h"

const char achmDNSQueryUbuntuServer[] =
{
  0x00, 0x00, //  Transaction ID
  0x00, 0x00, //  Flags
  0x00, 0x01, //  Number of questions
  0x00, 0x00, //  Number of answers
  0x00, 0x00, //  Number of authority resource records
  0x00, 0x00, //  Number of additional resource records
  0x0D, 0x75, 0x62, 0x75, 0x6e, 0x74, 0x75, 0x2d, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, //  "ubuntu-server"
  0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c,                                                 //  "local"
  0x00,       //  Terminator
  0x00, 0x01, //  Type(A record)
  0x00, 0x01  //  Class
};

const char achmDNSQuery_pione[] =
{
  0x00, 0x00, //  Transaction ID
  0x00, 0x00, //  Flags
  0x00, 0x01, //  Number of questions
  0x00, 0x00, //  Number of answers
  0x00, 0x00, //  Number of authority resource records
  0x00, 0x00, //  Number of additional resource records

  0x05, 0x70, 0x69, 0x6f, 0x6e, 0x65, //  "pione"
  0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, //  "local"
  0x00,       //  Terminator
  0x00, 0x01, //  Type(A record)
  0x00, 0x01, //  Class
};

const char achmDNSQuery_raspberrypi[] =
{
  0x00, 0x00, //  Transaction ID
  0x00, 0x00, //  Flags
  0x00, 0x01, //  Number of questions
  0x00, 0x00, //  Number of answers
  0x00, 0x00, //  Number of authority resource records
  0x00, 0x00, //  Number of additional resource records

  0x0B, 0x72, 0x61, 0x73, 0x70, 0x62, 0x65, 0x72, 0x72, 0x79, 0x70, 0x69, //  "raspberrypi"
  0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c,                                     //  "local"
  0x00,       //  Terminator
  0x00, 0x01, //  Type(A record)
  0x00, 0x01  //  Class
};


// --- New packet ---
static const uint8_t sc_abyDNSRequest[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x72, 0x61, 0x73,  //  .............ras
0x70, 0x62, 0x65, 0x72, 0x72, 0x79, 0x70, 0x69, 0x05, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x00, 0x00,  //  pberrypi.local..
0x01, 0x00, 0x01                                                                                 //  ...
};
// --- New packet ---
static const uint8_t sc_abyDNS0[] =
{
0x00, 0x00, 0x84, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x72, 0x61, 0x73,  //  .............ras
0x70, 0x62, 0x65, 0x72, 0x72, 0x79, 0x70, 0x69, 0x05, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x00, 0x00,  //  pberrypi.local..
0x01, 0x00, 0x01, 0x0F, 0x44, 0x45, 0x53, 0x4B, 0x54, 0x4F, 0x50, 0x2D, 0x32, 0x54, 0x54, 0x48,  //  ....DESKTOP-2TTH
0x50, 0x33, 0x42, 0xC0, 0x18, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x0E, 0x10, 0x00, 0x04, 0xC0,  //  P3B.............
0xA8, 0x4D, 0x84                                                                                 //  .M.
};

TEST(mDNS, Header0)
{
  SDNSHeader DNSHeader;
  uint32_t dwUsed = ParseDNSHeader(sc_abyDNS0, sizeof(sc_abyDNS0), &DNSHeader);
  EXPECT_EQ(12, dwUsed);
  EXPECT_EQ(1, DNSHeader.m_wQDCOUNT);
  EXPECT_EQ(1, DNSHeader.m_wANCOUNT);
}

TEST(mDNS, Response0)
{
  SDNSHeader DNSHeader;
  uint32_t dwUsed = ParseDNSHeader(sc_abyDNS0, sizeof(sc_abyDNS0), &DNSHeader);

  SDNSQuestion DNSQuestion;
  dwUsed = ParseDNSQuestion(sc_abyDNS0, dwUsed, sizeof(sc_abyDNS0), &DNSQuestion);
  EXPECT_EQ(35, dwUsed);

  SDNSCommonResourceRecord DNSCRR;
  dwUsed = ParseDNSDNSCommonResourceRecord(sc_abyDNS0, dwUsed, sizeof(sc_abyDNS0), &DNSCRR);
  EXPECT_EQ(63, dwUsed);

  uint8_t abyIP[4];
  dwUsed = ParseIPv4(sc_abyDNS0, dwUsed, sizeof(sc_abyDNS0), abyIP);
  EXPECT_EQ(67, dwUsed);
}

TEST(mDNS, Dump)
{
  DNSDumpPacket(sc_abyDNSRequest, sizeof(sc_abyDNSRequest));
}
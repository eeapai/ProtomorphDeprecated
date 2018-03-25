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
#ifndef __IO32_H__
#define __IO32_H__


///
/// User code controls LED
///  |
///  | Calls provided DoIO on given pointer to IHAL_IO32.
///  V
/// Custom communiation implementation of IHAL_IO32::DoIO  ( e.g. CHAL_IO32_UART_Impl::DoIO() )
///  |
///  | Sends bytes over communication channel.
///  V
/// Comm
///  :
///  : Data is transfered.
///  :
/// Comm
///  |
///  | Host side communication handler forwards bytes to local IHAL_IO32::DoIO implementation.
///  V
/// Custom HW IHAL_IO32::DoIO ( e.g. CHAL_IO32_RaspberryPI_Impl::DoIO() )
///  |
///  | Calls whatever is needed to make requested change on pins and reads their input levels.
///  V
/// Implementation specific functions

/// User code can be later moved on the other side of communication channel
/// and work directly with pointer to implementation specific IHAL_IO32 implementation.

struct IHAL_IO32
{
  enum EAction
  {
    pinMask = 0x1F,
    pinLSB = 0,
    pin0 = 0x00,
    pin1 = 0x01,
    pin2 = 0x02,
    pin3 = 0x03,
    pin4 = 0x04,
    pin5 = 0x05,
    pin6 = 0x06,
    pin7 = 0x07,
    pin8 = 0x08,
    pin9 = 0x09,
    pin10 = 0x0A,
    pin11 = 0x0B,
    pin12 = 0x0C,
    pin13 = 0x0D,
    pin14 = 0x0E,
    pin15 = 0x0F,
    pin16 = 0x10,
    pin17 = 0x11,
    pin18 = 0x12,
    pin19 = 0x13,
    pin20 = 0x14,
    pin21 = 0x15,
    pin22 = 0x16,
    pin23 = 0x17,
    pin24 = 0x18,
    pin25 = 0x19,
    pin26 = 0x1A,
    pin27 = 0x1B,
    pin28 = 0x1C,
    pin29 = 0x1D,
    pin30 = 0x1E,
    pin31 = 0x1F,
    numPins = 32,

    functionMask = 0x60,
    functionRead = 0x00,
    functionIn = 0x20,
    functionLow = 0x40,
    functionHigh = 0x60,

    responseLevelMask = 0x20,
    responseLevelLow = 0x00,
    responseLevelHigh = 0x20,

    reservedMask = 0x80
  };

  static EAction Pin(unsigned char byAction) { return (EAction)(byAction & pinMask); }
  static EAction Function(unsigned char byAction) { return (EAction)(byAction & functionMask); }
  static EAction GetReadLevel(unsigned char byResponse) { return (EAction)(byResponse & responseLevelMask); }
  static bool IsReadLevelHigh(unsigned char byResponse) { return responseLevelHigh == GetReadLevel(byResponse); }
  static bool IsReadLevelLow(unsigned char byResponse) { return responseLevelLow == GetReadLevel(byResponse); }

  static bool IsReturnOK(int nResponse) { return 0 <= nResponse; }
  ///
  /// Performs actions on pins. Call to request pin action(s).
  ///
  /// Called from protocol libraries or/and user code.
  /// Called from communication channel host side.
  /// Implement this to send pin actions over communication channel on client side.
  /// Implement this to provide IO actions on custom HW.
  /// 
  /// @return Number of reads.
  /// - negative FAIL
  /// - other OK, value is the number of reads;
  ///         is the byte count stored under pbyPinsOut;
  ///         repeated nNumReads if nNumReads given.
  ///
  virtual int DoIO(int nNum,                        ///< [in] Number of pin actions.
                   const unsigned char *pbyPinsIn,  ///< [in] Input pin actions. One byte per action.
                   unsigned char *pbyPinsOut,       ///< [out] Return pin info. One byte per functionRead action. Related to pin specified in input action.
                   int nNumReads = -1               ///< [in(optional)] Number of reads. Positive value returned on success.
                   ) = 0;

  static int NumberOfReads(int nNum,                        ///< [in] Number of pin actions.
                           const unsigned char *pbyPinsIn   ///< [in] Input pin actions. One byte per action.
                           )
  {
    int nReads = 0;
    for ( int PA = 0; PA < nNum; ++PA )
    {
      if ( functionRead == Function(pbyPinsIn[PA]) )
      {
        nReads++;
      }
    }
    return nReads;
  }

  static unsigned char ExtractByteMSBFirst(unsigned char *pbyData)
  {
    unsigned char byByte = 0;
    if ( IsReadLevelHigh(pbyData[0]) ) { byByte |= (1 << 7); }
    if ( IsReadLevelHigh(pbyData[1]) ) { byByte |= (1 << 6); }
    if ( IsReadLevelHigh(pbyData[2]) ) { byByte |= (1 << 5); }
    if ( IsReadLevelHigh(pbyData[3]) ) { byByte |= (1 << 4); }
    if ( IsReadLevelHigh(pbyData[4]) ) { byByte |= (1 << 3); }
    if ( IsReadLevelHigh(pbyData[5]) ) { byByte |= (1 << 2); }
    if ( IsReadLevelHigh(pbyData[6]) ) { byByte |= (1 << 1); }
    if ( IsReadLevelHigh(pbyData[7]) ) { byByte |= (1 << 0); }
    return byByte;
  }

  static unsigned char ExtractByteLSBFirst(unsigned char *pbyData)
  {
    unsigned char byByte = 0;
    if ( IsReadLevelHigh(pbyData[0]) ) { byByte |= (1 << 0); }
    if ( IsReadLevelHigh(pbyData[1]) ) { byByte |= (1 << 1); }
    if ( IsReadLevelHigh(pbyData[2]) ) { byByte |= (1 << 2); }
    if ( IsReadLevelHigh(pbyData[3]) ) { byByte |= (1 << 3); }
    if ( IsReadLevelHigh(pbyData[4]) ) { byByte |= (1 << 4); }
    if ( IsReadLevelHigh(pbyData[5]) ) { byByte |= (1 << 5); }
    if ( IsReadLevelHigh(pbyData[6]) ) { byByte |= (1 << 6); }
    if ( IsReadLevelHigh(pbyData[7]) ) { byByte |= (1 << 7); }
    return byByte;
  }
};

#endif


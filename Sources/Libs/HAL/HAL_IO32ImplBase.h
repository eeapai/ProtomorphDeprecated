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
#ifndef __HAL_IO32_IMPL_BASE_H__
#define __HAL_IO32_IMPL_BASE_H__

#include "HAL_IO32.h"

class CHAL_IO32_ImplBase : public IHAL_IO32
{
public:
  CHAL_IO32_ImplBase(unsigned long dwAllowedPinsMask = 0xFFFFFFFF, bool bRepeatSameOutput = false);
  virtual ~CHAL_IO32_ImplBase(){}
public:
  int DoIO(int nNum, const unsigned char *pbyPinsIn, unsigned char *pbyPinsOut, int nNumReads = -1) override;

protected:
  enum ELevel
  {
    levelLow,
    levelHigh
  };
  virtual bool writeValue(unsigned long dwPin, ELevel eLevel) = 0;
  virtual bool readValue(unsigned long dwPin, ELevel &reLevel) = 0;
  enum EDirection
  {
    directionIn,
    directionOut
  };
  virtual bool writeDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint = nullptr) = 0;

protected:
  bool IsPinAllowed(unsigned long dwPin) const { return m_dwAllowedPinsMask & (1 << dwPin); }
private:
  bool getResponseLevel(unsigned long dwPin, unsigned char &rbyResponseLevel);
  bool getLevel(unsigned long dwPin, ELevel &reLevel);
  bool setLevel(unsigned long dwPin, ELevel eLevel);
  bool getDirection(unsigned long dwPin, EDirection &reDirection);
  bool setDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint = nullptr);


private:
  unsigned long m_dwAllowedPinsMask = 0xFFFFFFFF;

  enum EPinState
  {
    stateUnknown = 0,
    stateIn,
    stateLow,
    stateHigh
  };

  EPinState m_aePinStates[numPins];

  bool m_bRepeatSameOutput = false;
};

#endif


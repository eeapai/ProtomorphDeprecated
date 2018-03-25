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
#include "HAL_IO32ImplBase.h"
#include <string.h>

CHAL_IO32_ImplBase::CHAL_IO32_ImplBase(unsigned long dwAllowedPinsMask, bool bRepeatSameOutput /*= false*/) :
  m_dwAllowedPinsMask(dwAllowedPinsMask),
  m_bRepeatSameOutput(bRepeatSameOutput)
{
  for ( unsigned long P = 0; P < sizeof(m_aePinStates) / sizeof(m_aePinStates[0]); ++P )
  {
    m_aePinStates[P] = stateUnknown;
  }
}


int CHAL_IO32_ImplBase::DoIO(int nNum, const unsigned char * pbyPinsIn, unsigned char * pbyPinsOut, int nNumReads /*= -1*/)
{
  bool bResult = true;
  unsigned long dwResponse = 0;
  unsigned char abyCachedSetFunction[numPins];
  if ( !m_bRepeatSameOutput )
  {
    memset(abyCachedSetFunction, functionRead, sizeof(abyCachedSetFunction));
  }
  for ( int nAction = 0; nAction < nNum; ++nAction )
  {
    unsigned long dwPin = IHAL_IO32::Pin(pbyPinsIn[nAction]);
    EAction eFunction = IHAL_IO32::Function(pbyPinsIn[nAction]);

    if ( !m_bRepeatSameOutput )
    {
      if ( IHAL_IO32::functionRead != eFunction )
      {
        if ( eFunction == abyCachedSetFunction[dwPin] )
        {
          continue;
        }
        abyCachedSetFunction[dwPin] = eFunction;
      }
    }
    switch ( eFunction )
    {
    case IHAL_IO32::functionRead  : bResult = getResponseLevel(dwPin, pbyPinsOut[dwResponse++]); break;
    case IHAL_IO32::functionIn    : bResult = setDirection(dwPin, directionIn); break;
    case IHAL_IO32::functionLow   : bResult = setLevel(dwPin, levelLow); break;
    case IHAL_IO32::functionHigh  : bResult = setLevel(dwPin, levelHigh); break;
    default: bResult = false; break;
    }
    if ( !bResult )
    {
      return -256 + pbyPinsIn[nAction];
    }
  }
  return dwResponse;
}

bool CHAL_IO32_ImplBase::getResponseLevel(unsigned long dwPin, unsigned char &rbyResponseLevel)
{
  rbyResponseLevel  = (unsigned char)(dwPin << IHAL_IO32::pinLSB);
  ELevel eLevel = levelLow;
  if ( getLevel(dwPin, eLevel) )
  {
    rbyResponseLevel |= (unsigned char)((( levelLow == eLevel ) ? IHAL_IO32::responseLevelLow : IHAL_IO32::responseLevelHigh));
    return true;
  }
  return false;
}

bool CHAL_IO32_ImplBase::getLevel(unsigned long dwPin, ELevel & reLevel)
{
  bool bResult = true;
  reLevel = ( m_aePinStates[dwPin] == stateHigh ) ? levelHigh : levelLow;
  EDirection ePinDirection = directionIn;
  if ( bResult )
  {
    bResult = getDirection(dwPin, ePinDirection);
  }

  if ( bResult && (directionIn == ePinDirection) )
  {
    bResult = readValue(dwPin, reLevel);
  }

  return bResult;
}

bool CHAL_IO32_ImplBase::setLevel(unsigned long dwPin, ELevel eLevel)
{
  bool bResult = true;
  if ( bResult )
  {
    bResult = setDirection(dwPin, directionOut, &eLevel);
  }
  if ( bResult )
  {
    bResult = writeValue(dwPin, eLevel);
  }
  if ( bResult )
  {
    m_aePinStates[dwPin] = (levelHigh == eLevel) ? stateHigh : stateLow;
  }
  return bResult;
}

bool CHAL_IO32_ImplBase::getDirection(unsigned long dwPin, EDirection & reDirection)
{
  reDirection = ( stateIn == m_aePinStates[dwPin] ) ? directionIn : directionOut;
  return stateUnknown != m_aePinStates[dwPin];
}

bool CHAL_IO32_ImplBase::setDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint /*= nullptr*/)
{
  bool bResult = true;
  EDirection ePinDirection = directionIn;
  if ( bResult )
  {
    bResult = getDirection(dwPin, ePinDirection);
  }

  if ( bResult )
  {
    if ( eDirection == ePinDirection )
    {
      return true;
    }
  }

  bResult = writeDirection(dwPin, eDirection, peOutLevelHint);

  if ( bResult )
  {
    if ( directionOut == eDirection )
    {
      if ( peOutLevelHint )
      {
        switch ( *peOutLevelHint )
        {
        case levelLow: m_aePinStates[dwPin] = stateLow; break;
        case levelHigh: m_aePinStates[dwPin] = stateHigh; break;
        default: break;
        }
      }
      else
      {
        bResult = false;
      }
    }
    else
    {
      m_aePinStates[dwPin] = stateIn;
    }
  }
  else
  {
    m_aePinStates[dwPin] = stateUnknown;
  }

  return bResult;
}



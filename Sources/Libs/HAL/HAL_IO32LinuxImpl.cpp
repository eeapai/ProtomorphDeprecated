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
#include "HAL_IO32LinuxImpl.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>

#define ASSERT(exp) assert(exp)

//static bool s_fileExist(const char *pcszPath)
//{
//  struct stat s;
//  return (stat(pcszPath, &s) == 0);
//}

class CTimer
{
public:
  CTimer(unsigned long dwDurationInMiliSeconds) :
    m_dwDurationInMiliSeconds(dwDurationInMiliSeconds),
    m_clockStart(clock()){}

  bool HasExpired()
  {
    clock_t clockNow = clock();
    clock_t clockDiff = clockNow - m_clockStart;
    int nClocksPerMiliSec = CLOCKS_PER_SEC / 1000;
    if ( !nClocksPerMiliSec )
    {
      return true;
    }
    unsigned long dwMiliSeconds = clockDiff / nClocksPerMiliSec;
    return dwMiliSeconds > m_dwDurationInMiliSeconds;
  }
private:
  unsigned long m_dwDurationInMiliSeconds;
  clock_t m_clockStart;
};

CHAL_IO32_LinuxImpl::CHAL_IO32_LinuxImpl(unsigned long dwAllowedPinsMask, bool bRepeatSameOutput /*= false*/) : base_class(dwAllowedPinsMask, bRepeatSameOutput)
{
}

CHAL_IO32_LinuxImpl::~CHAL_IO32_LinuxImpl()
{
  finalize();
}

void CHAL_IO32_LinuxImpl::Initialize()
{
  for ( unsigned long dwPin = 0; dwPin < numPins; ++dwPin )
  {
    if ( IsPinAllowed(dwPin) )
    {
      reserveGPIO(dwPin);
    }
  }
  for ( unsigned long dwPin = 0; dwPin < numPins; ++dwPin )
  {
    if ( IsPinAllowed(dwPin) )
    {
      unsigned char byPinToInput = (unsigned char)(functionIn | (dwPin << pinLSB));
      DoIO(1, &byPinToInput, nullptr, 0);
    }
  }
}

void CHAL_IO32_LinuxImpl::finalize()
{
  for ( unsigned long dwPin = 0; dwPin < numPins; ++dwPin )
  {
    if ( IsPinAllowed(dwPin) )
    {
      closeValue(dwPin);
      unreserveGPIO(dwPin);
    }
  }
}



bool CHAL_IO32_LinuxImpl::reserveGPIO(unsigned long dwPin)
{
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if ( 0 > fd )
  {
    return false;
  }
  char achPin[16] = { 0 };
  sprintf(achPin, "%d", dwPin);
  bool bResult = 0 < write(fd, achPin, strlen(achPin));
  close(fd);

  if ( bResult )
  {
    char achDirectionPath[48] = { 0 };
    sprintf(achDirectionPath, "/sys/class/gpio/gpio%d/direction", dwPin);

    bool bIsReserved = false;
    // Needs cca 60ms on RPi Zero W
    for ( CTimer TO(300); !TO.HasExpired(); )
    {
      int fDirection = open(achDirectionPath, O_WRONLY);
      bIsReserved = 0 < fDirection;
      close(fDirection);
      if ( bIsReserved )
      {
        break;
      }
    }
    bResult = bIsReserved;
  }
  return bResult;
}

bool CHAL_IO32_LinuxImpl::unreserveGPIO(unsigned long dwPin)
{
  int fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if ( 0 > fd )
  {
    return false;
  }
  char achPin[16] = { 0 };
  sprintf(achPin, "%d", dwPin);
  bool bResult = 0 < write(fd, achPin, strlen(achPin));
  close(fd);
  return bResult;
}

bool CHAL_IO32_LinuxImpl::writeValue(unsigned long dwPin, ELevel eLevel)
{
  bool bResult = true;

  if ( bResult && !isValueOpened(dwPin) )
  {
    bResult = openValue(dwPin, directionOut);
  }

  int fPinValue = m_afValues[dwPin];
  if ( bResult )
  {
    bResult = 0 < write(fPinValue, ( levelHigh == eLevel ) ? "1" : "0", 1);
  }
  if ( bResult )
  {
    bResult = 0 <= lseek(fPinValue, 0, SEEK_SET);
  }

  return bResult;
}

bool CHAL_IO32_LinuxImpl::readValue(unsigned long dwPin, ELevel & reLevel)
{
  bool bResult = true;

  if ( bResult && !isValueOpened(dwPin) )
  {
    bResult = openValue(dwPin, directionIn);
  }

  int fPinValue = m_afValues[dwPin];
  char chValue = 0;
  if ( bResult )
  {
    bResult = 0 < read(fPinValue, &chValue, 1);
  }

  if ( bResult )
  {
    reLevel = ('0' == chValue) ? levelLow : levelHigh;
  }

  if ( bResult )
  {
    bResult = 0 <= lseek(fPinValue, 0, SEEK_SET);
  }
  return bResult;
}

bool CHAL_IO32_LinuxImpl::writeDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint /*= nullptr*/)
{
  bool bResult = true;

  if ( bResult && isValueOpened(dwPin) )
  {
    bResult = closeValue(dwPin);
  }

  char achDirectionPath[48] = { 0 };
  sprintf(achDirectionPath, "/sys/class/gpio/gpio%d/direction", dwPin);

  int fDirection = open(achDirectionPath, O_WRONLY);
  if ( bResult )
  {
    bResult = 0 < fDirection;
  }
  ASSERT(bResult);
  if ( bResult )
  {
    if ( directionOut == eDirection )
    {
      const char *pcszOutVal = "out";
      if ( peOutLevelHint )
      {
        switch ( *peOutLevelHint )
        {
        case levelLow: pcszOutVal = "low"; break;
        case levelHigh: pcszOutVal = "high"; break;
        default: break;
        }
        bResult = 0 < write(fDirection, pcszOutVal, strlen(pcszOutVal));
      }
      else
      {
        bResult = false;
      }
    }
    else
    {
      bResult = 0 < write(fDirection, "in", 2);
    }
    close(fDirection);
  }
  ASSERT(bResult);

  if ( bResult )
  {
    bResult = openValue(dwPin, eDirection);
  }

  return bResult;
}

bool CHAL_IO32_LinuxImpl::openValue(unsigned long dwPin, EDirection eDirection)
{
  bool bResult = true;
  char achValuePath[48] = { 0 };
  sprintf(achValuePath, "/sys/class/gpio/gpio%d/value", dwPin);

  if ( isValueOpened(dwPin) )
  {
    bResult = closeValue(dwPin);
  }

  int fValue = -1;
  if ( bResult )
  {
    fValue = open(achValuePath, ( directionIn == eDirection ) ? O_RDONLY : O_WRONLY);
    bResult = 0 < fValue;
  }

  if ( bResult )
  {
    m_afValues[dwPin] = fValue;
  }
  return bResult;
}

bool CHAL_IO32_LinuxImpl::isValueOpened(unsigned long dwPin) const
{
  return m_afValues[dwPin] > 0;
}

bool CHAL_IO32_LinuxImpl::closeValue(unsigned long dwPin)
{
  if ( !isValueOpened(dwPin) )
  {
    return true;
  }
  close(m_afValues[dwPin]);
  m_afValues[dwPin] = -1;
  return true;
}



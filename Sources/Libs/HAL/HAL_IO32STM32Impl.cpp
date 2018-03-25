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
#include "Defs.h"
#include "HAL_IO32STM32Impl.h"

static unsigned long s_enabledPinsFromConf(const CHAL_IO32_STM32Impl::SInitConf *pConf)
{
  unsigned long dwEnabledPins = 0;
  for ( int P = 0; P < IHAL_IO32::numPins; ++P )
  {
    if ( pConf->m_aPins[P].m_pPort )
    {
      dwEnabledPins |= (1 << P);
    }
  }
  return dwEnabledPins;
}
CHAL_IO32_STM32Impl::CHAL_IO32_STM32Impl(const CHAL_IO32_STM32Impl::SInitConf *pConf, bool bRepeatSameOutput) : base_class(s_enabledPinsFromConf(pConf), bRepeatSameOutput)
{
  for ( int P = 0; P < IHAL_IO32::numPins; ++P )
  {
    m_apPorts[P] = pConf->m_aPins[P].m_pPort;
    m_aPins[P] = pConf->m_aPins[P].m_sPin;
    if ( IsPinAllowed(P) )
    {
      unsigned char byPinToInput = (unsigned char)(functionIn | (P << pinLSB));
      DoIO(1, &byPinToInput, nullptr, 0);
    }
  }
}

bool CHAL_IO32_STM32Impl::writeValue(unsigned long dwPin, ELevel eLevel)
{
  HAL_GPIO_WritePin(m_apPorts[dwPin], m_aPins[dwPin], (levelHigh == eLevel) ? GPIO_PIN_SET : GPIO_PIN_RESET );
  return true;
}

bool CHAL_IO32_STM32Impl::readValue(unsigned long dwPin, ELevel& reLevel)
{
  reLevel = (GPIO_PIN_SET == HAL_GPIO_ReadPin(m_apPorts[dwPin], m_aPins[dwPin])) ? levelHigh : levelLow;
  return true;
}

bool CHAL_IO32_STM32Impl::writeDirection(unsigned long dwPin, EDirection eDirection, ELevel* peOutLevelHint)
{
  GPIO_InitTypeDef gpioInit = { 0 };
  gpioInit.Pin = m_aPins[dwPin];
  if ( directionOut == eDirection )
  {
    writeValue(dwPin, *peOutLevelHint);
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
  }
  else
  {
    gpioInit.Mode = GPIO_MODE_INPUT;
  }
  gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
  gpioInit.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(m_apPorts[dwPin], &gpioInit);
  return true;
}

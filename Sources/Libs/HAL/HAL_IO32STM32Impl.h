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
#ifndef __HAL_IO32_STM32_IMPL_H__
#define __HAL_IO32_STM32_IMPL_H__

#include "HAL_IO32ImplBase.h"

typedef GPIO_TypeDef GPIO_TypeDef;

class CHAL_IO32_STM32Impl : public CHAL_IO32_ImplBase
{
  typedef CHAL_IO32_ImplBase base_class;
public:
  struct SInitConf
  {
    struct SPinGPIO
    {
      GPIO_TypeDef *m_pPort;
      unsigned short m_sPin;
    };
    SPinGPIO m_aPins[numPins];
  };

  CHAL_IO32_STM32Impl(const SInitConf *pConf, bool bRepeatSameOutput = false);
protected:
  bool writeValue(unsigned long dwPin, ELevel eLevel) override;
  bool readValue(unsigned long dwPin, ELevel &reLevel) override;
  bool writeDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint = nullptr) override;

private:
  GPIO_TypeDef *m_apPorts[IHAL_IO32::numPins];
  unsigned short m_aPins[IHAL_IO32::numPins];
};

#endif


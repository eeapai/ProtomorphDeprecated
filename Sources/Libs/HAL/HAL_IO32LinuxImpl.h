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
#ifndef __HAL_IO32_LINUX_IMPL_H__
#define __HAL_IO32_LINUX_IMPL_H__

#include "HAL_IO32ImplBase.h"

class CHAL_IO32_LinuxImpl : public CHAL_IO32_ImplBase
{
  typedef CHAL_IO32_ImplBase base_class;
public:
  CHAL_IO32_LinuxImpl(unsigned long dwAllowedPinsMask = 0xFFFFFFFF, bool bRepeatSameOutput = false);
  ~CHAL_IO32_LinuxImpl();

  void Initialize();

private:
  void finalize();

protected:
  bool writeValue(unsigned long dwPin, ELevel eLevel) override;
  bool readValue(unsigned long dwPin, ELevel &reLevel) override;
  bool writeDirection(unsigned long dwPin, EDirection eDirection, ELevel *peOutLevelHint = nullptr) override;

private:

  static bool reserveGPIO(unsigned long dwPin);
  static bool unreserveGPIO(unsigned long dwPin);

  bool openValue(unsigned long dwPin, EDirection eDirection);
  bool isValueOpened(unsigned long dwPin) const;
  bool closeValue(unsigned long dwPin);

private:
  int m_afValues[numPins] = { -1 };

};

#endif


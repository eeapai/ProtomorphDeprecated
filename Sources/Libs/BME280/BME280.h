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
#ifndef __BME280_H__
#define __BME280_H__

class CBME280
{
public:

  static const unsigned char sc_byBME280_I2C_ADDRESS_SDO_LOW = 0x76;
  static const unsigned char sc_byBME280_I2C_ADDRESS_SDO_HIGH = 0x77;

  struct II2CCallback
  {
    virtual unsigned char BME280_I2CStartReadStop(unsigned char byI2CAddress, unsigned char byFirstRegAddress, unsigned char *pbyDestData, unsigned char byNumBytes) = 0;
    virtual unsigned char BME280_I2CStartWriteStop(unsigned char byI2CAddress, unsigned char byFirstRegAddress, const unsigned char *pbySrcData, unsigned char byNumBytes) = 0;
  };

  unsigned char Init(unsigned char byI2CAddress, II2CCallback *pI2C); // Must wait 2ms after power up before calling
  void TriggerMeasurement() const;
  unsigned char IsMeasurementInProgress() const;
  static const unsigned char sc_byBME280_PRESS_DECIMAL_PLACES = 2;
  static const unsigned char sc_byBME280_TEMP_DECIMAL_PLACES = 2;
  static const unsigned char sc_byBME280_HUM_DECIMAL_PLACES = 3;
  void ReadMeasuredResults(long *plTemp, unsigned long *pdwPress, unsigned long *pdwRelHum);
  static void InsertDecimalDot(const char *pcszSrc, char *pcszDest, int nBeforeNumPlaces);
  static unsigned char FormatWithDecimalDot(long lVal, void *pDest, int nBeforeNumPlaces);

private:
  void writeReg(unsigned char byAddress, unsigned char byValue) const;
  void readReg(unsigned char byAddress, unsigned char *pbyValue) const;
  long compensateTemp(signed long lTempRaw);
  unsigned long compensatePress(signed long lPressRaw) const;
  unsigned long compensateHum(signed long lHumRaw) const;
  unsigned char i2cStartReadStop(unsigned char byAddress, unsigned char *pbyDestData, unsigned char byNumBytes) const;
  unsigned char i2cStartWriteStop(unsigned char byAddress, const unsigned char *pbySrcData, unsigned char byNumBytes) const;


private:
  unsigned char m_byI2CAddress;
  II2CCallback *m_pi2c = nullptr;

  unsigned short m_wDIG_T1;   // 0x88 / 0x89 [7:0] / [15:8] 
  signed short   m_sDIG_T2;   // 0x8A / 0x8B [7:0] / [15:8] 
  signed short   m_sDIG_T3;   // 0x8C / 0x8D [7:0] / [15:8] 
  unsigned short m_wDIG_P1;   // 0x8E / 0x8F [7:0] / [15:8] 
  signed short   m_sDIG_P2;   // 0x90 / 0x91 [7:0] / [15:8] 
  signed short   m_sDIG_P3;   // 0x92 / 0x93 [7:0] / [15:8] 
  signed short   m_sDIG_P4;   // 0x94 / 0x95 [7:0] / [15:8] 
  signed short   m_sDIG_P5;   // 0x96 / 0x97 [7:0] / [15:8] 
  signed short   m_sDIG_P6;   // 0x98 / 0x99 [7:0] / [15:8] 
  signed short   m_sDIG_P7;   // 0x9A / 0x9B [7:0] / [15:8] 
  signed short   m_sDIG_P8;   // 0x9C / 0x9D [7:0] / [15:8] 
  signed short   m_sDIG_P9;   // 0x9E / 0x9F [7:0] / [15:8] 
  unsigned char  m_byDIG_H1;  // 0xA1 [7:0] 
  signed short   m_sDIG_H2;   // 0xE1 / 0xE2 [7:0] / [15:8] 
  unsigned char  m_byDIG_H3;  // 0xE3 [7:0] 
  signed short   m_sDIG_H4;   // 0xE4 / 0xE5[3:0] [11:4] / [3:0] 
  signed short   m_sDIG_H5;   // 0xE5[7:4] / 0xE6  [3:0] / [11:4] 
  signed char    m_cDIG_H6;   // 0xE7

  long           m_lTFine;

  unsigned long m_lStartupLoops;
};



#endif

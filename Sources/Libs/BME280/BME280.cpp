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
#include "BME280.h"
#include <string.h>
#include <stdio.h>

#ifdef BIG_ENDIAN
#else
#define LSB2WORD(buff) (*(unsigned short *)(buff))
#define LSB2SHORT(buff) (*(short *)(buff))
#endif

#define REG_ADR_ID            0xD0

#define REG_ADR_DIG_T1_LO     0x88
#define REG_ADR_DIG_T1_HI     0x89
#define REG_ADR_DIG_T2_LO     0x8A
#define REG_ADR_DIG_T2_HI     0x8B
#define REG_ADR_DIG_T3_LO     0x8C
#define REG_ADR_DIG_T3_HI     0x8D
#define REG_ADR_DIG_P1_LO     0x8E
#define REG_ADR_DIG_P1_HI     0x8F
#define REG_ADR_DIG_P2_LO     0x90
#define REG_ADR_DIG_P2_HI     0x91
#define REG_ADR_DIG_P3_LO     0x92
#define REG_ADR_DIG_P3_HI     0x93
#define REG_ADR_DIG_P4_LO     0x94
#define REG_ADR_DIG_P4_HI     0x95
#define REG_ADR_DIG_P5_LO     0x96
#define REG_ADR_DIG_P5_HI     0x97
#define REG_ADR_DIG_P6_LO     0x98
#define REG_ADR_DIG_P6_HI     0x99
#define REG_ADR_DIG_P7_LO     0x9A
#define REG_ADR_DIG_P7_HI     0x9B
#define REG_ADR_DIG_P8_LO     0x9C
#define REG_ADR_DIG_P8_HI     0x9D
#define REG_ADR_DIG_P9_LO     0x9E
#define REG_ADR_DIG_P9_HI     0x9F
#define REG_ADR_DIG_H1        0xA1
#define REG_ADR_DIG_H2_LO     0xE1
#define REG_ADR_DIG_H2_HI     0xE2
#define REG_ADR_DIG_H3        0xE3
#define REG_ADR_DIG_H4_HI     0xE4
#define REG_ADR_DIG_H4_LO_H5_LO 0xE5
#define REG_ADR_DIG_H5_HI     0xE6
#define REG_ADR_DIG_H6        0xE7

enum ECalibrationRegs
{
  calibregDIG_T1_LO,
  calibregDIG_T1_HI,
  calibregDIG_T2_LO,
  calibregDIG_T2_HI,
  calibregDIG_T3_LO,
  calibregDIG_T3_HI,
  calibregDIG_P1_LO,
  calibregDIG_P1_HI,
  calibregDIG_P2_LO,
  calibregDIG_P2_HI,
  calibregDIG_P3_LO,
  calibregDIG_P3_HI,
  calibregDIG_P4_LO,
  calibregDIG_P4_HI,
  calibregDIG_P5_LO,
  calibregDIG_P5_HI,
  calibregDIG_P6_LO,
  calibregDIG_P6_HI,
  calibregDIG_P7_LO,
  calibregDIG_P7_HI,
  calibregDIG_P8_LO,
  calibregDIG_P8_HI,
  calibregDIG_P9_LO,
  calibregDIG_P9_HI,
  calibregDIG_H1   ,
  calibregDIG_H2_LO,
  calibregDIG_H2_HI,
  calibregDIG_H3   ,
  calibregDIG_H4_HI,
  calibregDIG_H4_LO_H5_LO,
  calibregDIG_H5_HI,
  calibregDIG_H6   ,

  calibregNumRegs
};

#define REG_ADR_ID            0xD0
#define REG_ADR_RESET         0xE0
#define REG_ADR_CTRL_HUM      0xF2
#define REG_ADR_STATUS        0xF3
#define REG_ADR_CTRL_MEAS     0xF4
#define REG_ADR_CONFIG        0xF5
#define REG_ADR_PRESS_MSB     0xF7
#define REG_ADR_PRESS_LSB     0xF8
#define REG_ADR_PRESS_XLSB    0xF9
#define REG_ADR_TEMP_MSB      0xFA
#define REG_ADR_TEMP_LSB      0xFB
#define REG_ADR_TEMP_XLSB     0xFC
#define REG_ADR_HUM_MSB       0xFD
#define REG_ADR_HUM_LSB       0xFE

unsigned char CBME280::Init(unsigned char byI2CAddress, CBME280::II2CCallback *pI2C)
{
  unsigned char abyCalibRegs[calibregNumRegs] = {0};
  unsigned char byResult = 1;
  unsigned char byID = 0xFF;
  m_byI2CAddress = byI2CAddress;
  m_lStartupLoops = 0;
  m_pi2c = pI2C;

  while ( byResult && ( ( 0x00 == byID ) || ( 0xFF == byID ) ) )
  {
    byResult = i2cStartReadStop(REG_ADR_ID, &byID, 1);
    m_lStartupLoops++;
  }

  byResult = i2cStartReadStop(REG_ADR_DIG_T1_LO, abyCalibRegs, calibregDIG_P9_HI - calibregDIG_T1_LO + 1);
  if ( !byResult )
  {
    byResult = i2cStartReadStop(REG_ADR_DIG_H1, &abyCalibRegs[calibregDIG_H1], calibregDIG_H1 - calibregDIG_H1 + 1);
  }
  if ( !byResult )
  {
    byResult = i2cStartReadStop(REG_ADR_DIG_H2_LO, &abyCalibRegs[calibregDIG_H2_LO], calibregDIG_H6 - calibregDIG_H2_LO + 1);
  }


  m_wDIG_T1  = LSB2WORD(&abyCalibRegs[calibregDIG_T1_LO]);
  m_sDIG_T2  = LSB2SHORT(&abyCalibRegs[calibregDIG_T2_LO]);
  m_sDIG_T3  = LSB2SHORT(&abyCalibRegs[calibregDIG_T3_LO]);
  m_wDIG_P1  = LSB2WORD(&abyCalibRegs[calibregDIG_P1_LO]);
  m_sDIG_P2  = LSB2SHORT(&abyCalibRegs[calibregDIG_P2_LO]);
  m_sDIG_P3  = LSB2SHORT(&abyCalibRegs[calibregDIG_P3_LO]);
  m_sDIG_P4  = LSB2SHORT(&abyCalibRegs[calibregDIG_P4_LO]);
  m_sDIG_P5  = LSB2SHORT(&abyCalibRegs[calibregDIG_P5_LO]);
  m_sDIG_P6  = LSB2SHORT(&abyCalibRegs[calibregDIG_P6_LO]);
  m_sDIG_P7  = LSB2SHORT(&abyCalibRegs[calibregDIG_P7_LO]);
  m_sDIG_P8  = LSB2SHORT(&abyCalibRegs[calibregDIG_P8_LO]);
  m_sDIG_P9  = LSB2SHORT(&abyCalibRegs[calibregDIG_P9_LO]);
  m_byDIG_H1 = abyCalibRegs[calibregDIG_H1];
  m_sDIG_H2  = LSB2SHORT(&abyCalibRegs[calibregDIG_H2_LO]);
  m_byDIG_H3 = abyCalibRegs[calibregDIG_H3];
  m_sDIG_H4  = abyCalibRegs[calibregDIG_H4_HI];
  m_sDIG_H4  = m_sDIG_H4 << 4;
  m_sDIG_H4  = m_sDIG_H4 | (abyCalibRegs[calibregDIG_H4_LO_H5_LO] & 0xF);
  m_sDIG_H5  = LSB2SHORT(&abyCalibRegs[calibregDIG_H4_LO_H5_LO]) >> 4;
  m_cDIG_H6  = (signed char)abyCalibRegs[calibregDIG_H6];

  return byResult;
}

void CBME280::writeReg(unsigned char byAddress, unsigned char byValue) const
{
  unsigned char byResult = 0;
  byResult = i2cStartWriteStop(byAddress, &byValue, 1);
}

void CBME280::readReg(unsigned char byAddress, unsigned char *pbyValue) const
{
  unsigned char byResult = 0;
  byResult = i2cStartReadStop(byAddress, pbyValue, 1);
}

long CBME280::compensateTemp(signed long lTempRaw)
{
  long lX1 = ((((lTempRaw >> 3) - (((long)m_wDIG_T1) << 1))) * ((long)m_sDIG_T2)) >> 11;
  long lX2 = (((((lTempRaw >> 4) - ((long)m_wDIG_T1)) * ((lTempRaw >> 4) - ((long)m_wDIG_T1))) >> 12) * ((long)m_sDIG_T3)) >> 14;
  m_lTFine = lX1 + lX2;
  return (m_lTFine * 5 + 128) >> 8;
}

unsigned long CBME280::compensatePress(signed long lPressRaw) const
{
  unsigned long v_pressure_u32 = 0;

  long lX1 = (m_lTFine >> 1) - (long)64000;
  long lX2 = (((lX1 >> 2) * (lX1 >> 2)) >> 11) * ((long)(m_sDIG_P6));
  lX2 = lX2 + ((lX1 *((long)m_sDIG_P5)) << 1);
  lX2 = (lX2 >> 2) + (((long)m_sDIG_P4) << 16);
  lX1 = (((m_sDIG_P3 * (((lX1 >> 2) * (lX1 >> 2)) >> 13)) >> 3) + ((((long)m_sDIG_P2) * lX1) >> 1)) >> 18;
  lX1 = ((((32768 + lX1)) * ((long)m_wDIG_P1)) >> 15);
  v_pressure_u32 = (((unsigned long)(((long)1048576) - lPressRaw) - (lX2 >> 12))) * 3125;
  if ( v_pressure_u32 < 0x80000000 )
  {
    if ( lX1 != 0 )
    {
      v_pressure_u32 = (v_pressure_u32 << 1) / ((unsigned long)lX1);
    }
    else
    {
      return 0;
    }
  }
  else
  {
    if ( lX1 != 0 )
    {
      v_pressure_u32 = (v_pressure_u32 / (unsigned long)lX1) * 2;
    }
    else
    {
      return 0;
    }
  }

  lX1 = (((long)m_sDIG_P9) * ((long)(((v_pressure_u32 >> 3) * (v_pressure_u32 >> 3)) >> 13))) >> 12;
  lX2 = (((long)(v_pressure_u32 >> 2)) * ((long)m_sDIG_P8)) >> 13;
  v_pressure_u32 = (unsigned long)((long)v_pressure_u32 + ((lX1 + lX2 + m_sDIG_P7) >> 4));

  return v_pressure_u32;
}

unsigned long CBME280::compensateHum(signed long lHumRaw) const
{
  long lX1 = (m_lTFine - ((long)76800));
  lX1 = (((((lHumRaw << 14) - (((long)m_sDIG_H4) << 20) - (((long)m_sDIG_H5) * lX1)) + 
    ((long)16384)) >> 15) * (((((((lX1 * ((long)m_cDIG_H6)) >> 10) * (((lX1 * ((long)m_byDIG_H3)) >> 11) + 
    ((long)32768))) >> 10) + ((long)2097152)) * ((long)m_sDIG_H2) + 8192) >> 14));
  lX1 = (lX1 - (((((lX1 >> 15) *(lX1 >> 15)) >> 7) * ((long)m_byDIG_H1)) >> 4));
  lX1 = (lX1 < 0) ? 0 : lX1;
  lX1 = (lX1 > 419430400) ? 419430400 : lX1;
  return (unsigned long)(lX1 >> 12);
}

unsigned char CBME280::i2cStartReadStop(unsigned char byAddress, unsigned char * pbyDestData, unsigned char byNumBytes) const
{
  if ( m_pi2c )
  {
    return m_pi2c->BME280_I2CStartReadStop(m_byI2CAddress, byAddress, pbyDestData, byNumBytes);
  }
  return 0xAB;
}

unsigned char CBME280::i2cStartWriteStop(unsigned char byAddress, const unsigned char * pbySrcData, unsigned char byNumBytes) const
{
  if ( m_pi2c )
  {
    return m_pi2c->BME280_I2CStartWriteStop(m_byI2CAddress, byAddress, pbySrcData, byNumBytes);
  }
  return 0xAB;
}

#define REG_ADR_CTRL_HUM        0xF2
#define CTRL_HUM_OVERSAMPLING_SKIPPED 0x00
#define CTRL_HUM_OVERSAMPLING_X1      0x01
#define CTRL_HUM_OVERSAMPLING_X2      0x02
#define CTRL_HUM_OVERSAMPLING_X4      0x03
#define CTRL_HUM_OVERSAMPLING_X8      0x04
#define CTRL_HUM_OVERSAMPLING_X16     0x05

#define REG_ADR_CTRL_MEAS       0xF4
#define CTRL_MEAS_TEMP_OVERSAMPLING_SKIPPED 0x00
#define CTRL_MEAS_TEMP_OVERSAMPLING_X1      0x20
#define CTRL_MEAS_TEMP_OVERSAMPLING_X2      0x40
#define CTRL_MEAS_TEMP_OVERSAMPLING_X4      0x60
#define CTRL_MEAS_TEMP_OVERSAMPLING_X8      0x80
#define CTRL_MEAS_TEMP_OVERSAMPLING_X16     0xA0

#define CTRL_MEAS_PRESS_OVERSAMPLING_SKIPPED 0x00
#define CTRL_MEAS_PRESS_OVERSAMPLING_X1      0x04
#define CTRL_MEAS_PRESS_OVERSAMPLING_X2      0x08
#define CTRL_MEAS_PRESS_OVERSAMPLING_X4      0x0C
#define CTRL_MEAS_PRESS_OVERSAMPLING_X8      0x10
#define CTRL_MEAS_PRESS_OVERSAMPLING_X16     0x14

#define CTRL_MEAS_MODE_SLEEP      0x00
#define CTRL_MEAS_MODE_FORCED     0x01
#define CTRL_MEAS_MODE_NORMAL     0x03
#define CTRL_MEAS_MODE_MASK       0x03

enum ERawDataReg
{
  rawPressMSB,
  rawPressLSB,
  rawPressXLSB,
  rawTempMSB,
  rawTempLSB,
  rawTempXLSB,
  rawHumMSB,
  rawHumLSB,

  rawNumRegs
};

void CBME280::TriggerMeasurement() const
{
  writeReg(REG_ADR_CTRL_MEAS, CTRL_MEAS_TEMP_OVERSAMPLING_X1 | CTRL_MEAS_PRESS_OVERSAMPLING_X1);
  writeReg(REG_ADR_CTRL_HUM, CTRL_HUM_OVERSAMPLING_X1);
  writeReg(REG_ADR_CTRL_MEAS, CTRL_MEAS_TEMP_OVERSAMPLING_X1 | CTRL_MEAS_PRESS_OVERSAMPLING_X1 | CTRL_MEAS_MODE_FORCED);
}
unsigned char CBME280::IsMeasurementInProgress() const
{
  unsigned char byCTRL_MEAS = 0;
  readReg(REG_ADR_CTRL_MEAS, &byCTRL_MEAS);
  return ( CTRL_MEAS_MODE_SLEEP != (byCTRL_MEAS & CTRL_MEAS_MODE_MASK) ) ? 1 : 0;
}

#define REG_ADR_PRESS_MSB 0xF7

void CBME280::ReadMeasuredResults(long *plTemp, unsigned long *pdwPress, unsigned long *pdwRelHum)
{
  long lTempFine = 0;
  unsigned char abyDataRaw[rawNumRegs] = {0};
  long lPressRaw = 0;
  long lHumRaw = 0;
  long lTempRaw = 0;

  i2cStartReadStop(REG_ADR_PRESS_MSB, abyDataRaw, rawNumRegs);

  lPressRaw = abyDataRaw[rawPressMSB];
  lPressRaw <<= 8;
  lPressRaw |= abyDataRaw[rawPressLSB];
  lPressRaw <<= 8;
  lPressRaw |= abyDataRaw[rawPressXLSB];
  lPressRaw >>= 4;

  lTempRaw = abyDataRaw[rawTempMSB];
  lTempRaw <<= 8;
  lTempRaw |= abyDataRaw[rawTempLSB];
  lTempRaw <<= 8;
  lTempRaw |= abyDataRaw[rawTempXLSB];
  lTempRaw >>= 4;

  lHumRaw = abyDataRaw[rawHumMSB];
  lHumRaw <<= 8;
  lHumRaw |= abyDataRaw[rawHumLSB];

  *plTemp = compensateTemp(lTempRaw);
  *pdwPress = compensatePress(lPressRaw);
  *pdwRelHum = compensateHum(lHumRaw);
}

void CBME280::InsertDecimalDot(const char *pcszSrc, char *pcszDest, int nBeforeNumPlaces)
{
  size_t nLength = strlen(pcszSrc);
  pcszDest[nLength + 1] = 0;
  memmove(pcszDest, pcszSrc, nLength - nBeforeNumPlaces);
  pcszDest[nLength - nBeforeNumPlaces] = '.';
  memmove(pcszDest + nLength - nBeforeNumPlaces + 1, pcszSrc + nLength - nBeforeNumPlaces, nBeforeNumPlaces);
}

unsigned char CBME280::FormatWithDecimalDot(long lVal, void *pDest, int nBeforeNumPlaces)
{
  char achData[16] = "000000000000000";
  char *pszData = &achData[nBeforeNumPlaces];
  int nLength = sprintf(pszData, "%d", lVal);
  unsigned char *pcszDest = (unsigned char *)pDest;
  pcszDest[nBeforeNumPlaces + nLength + 1] = 0;

  if ( nBeforeNumPlaces >= nLength )
  {
    pszData -= nBeforeNumPlaces - nLength + 1;
    nLength = nBeforeNumPlaces + 1;
  }

  memmove(pcszDest, pszData, nLength - nBeforeNumPlaces);
  pcszDest[nLength - nBeforeNumPlaces] = '.';
  memmove(pcszDest + nLength - nBeforeNumPlaces + 1, &pszData[nLength - nBeforeNumPlaces], nBeforeNumPlaces);
  return nBeforeNumPlaces ? nLength + 1 : nLength;
}
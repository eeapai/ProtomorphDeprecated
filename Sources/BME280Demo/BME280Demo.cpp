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
#include <stdio.h>
#include <Windows.h>
//#define C_BME // uncomment to test old C style BME280 lib
#include "WinCommFactory.h"
#include "HAL_IO32ICommProxy.h"
#include "BME280.h"
#ifdef C_BME
#include "BME280HAL.h"

static const unsigned char sc_byBME280GNDOff = IHAL_IO32::functionIn | IHAL_IO32::pin27;
static const unsigned char sc_byBME280DataRead = IHAL_IO32::functionRead | IHAL_IO32::pin25;
static const unsigned char sc_byBME280DataHigh = IHAL_IO32::functionIn | IHAL_IO32::pin25;
static const unsigned char sc_byBME280DataLow = IHAL_IO32::functionLow | IHAL_IO32::pin25;
static const unsigned char sc_byBME280ClockHigh = IHAL_IO32::functionIn | IHAL_IO32::pin26;
static const unsigned char sc_byBME280ClockLow = IHAL_IO32::functionLow | IHAL_IO32::pin26;
#endif

class CBME280I2CCallbackIO32Impl : public CBME280::II2CCallback
{
public:
  CBME280I2CCallbackIO32Impl(IHAL_IO32 *pIO32, unsigned char byDataPin, unsigned char byClockPin) :
    m_pIO32(pIO32)
  {
    m_byBME280DataRead = IHAL_IO32::functionRead | byDataPin;
    m_byBME280DataHigh = IHAL_IO32::functionIn | byDataPin;
    m_byBME280DataLow = IHAL_IO32::functionLow | byDataPin;
    m_byBME280ClockHigh = IHAL_IO32::functionIn | byClockPin;
    m_byBME280ClockLow = IHAL_IO32::functionLow | byClockPin;
  }

  // Inherited via II2CCallback
  unsigned char BME280_I2CStartReadStop(unsigned char byI2CAddress, unsigned char byFirstRegAddress, unsigned char * pbyDestData, unsigned char byNumBytes) override
  {
    unsigned char abyIO32CmdBuff[1024];
    unsigned char abyIO32RetBuff[1024];
    int nIO32Cmd = 0;
    unsigned char *pbyIO32Cmd = abyIO32CmdBuff;
    unsigned char *pbyIO32Ret = abyIO32RetBuff;
    const bool bDoACK = true;
    const bool bReadACK = false;

    nIO32Cmd += addStart(pbyIO32Cmd + nIO32Cmd);
    nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, byI2CAddress << 1, bReadACK);
    nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, byFirstRegAddress, bReadACK);
    nIO32Cmd += addStart(pbyIO32Cmd + nIO32Cmd);
    nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, (byI2CAddress << 1) | 1, bReadACK);

    for ( int n = 0; n < byNumBytes; ++n )
    {
      nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, 0xFF, n < (byNumBytes - 1));
    }

    nIO32Cmd += addStop(pbyIO32Cmd + nIO32Cmd);

    m_pIO32->DoIO(nIO32Cmd, pbyIO32Cmd, pbyIO32Ret, 9 * (3 + byNumBytes));

    bool bOK = true;
    int nRet = 8;
    bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
    nRet += 9;
    bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
    nRet += 9;
    bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
    nRet += 1;

    for ( int n = 0; n < byNumBytes; ++n )
    {
      *pbyDestData = IHAL_IO32::ExtractByteMSBFirst(pbyIO32Ret + nRet);
      pbyDestData++;
      nRet += 9;
    }

    return 0;
  }

  unsigned char BME280_I2CStartWriteStop(unsigned char byI2CAddress, unsigned char byFirstRegAddress, const unsigned char * pbySrcData, unsigned char byNumBytes) override
  {
    unsigned char abyIO32CmdBuff[1024];
    unsigned char abyIO32RetBuff[1024];
    int nIO32Cmd = 0;
    unsigned char *pbyIO32Cmd = abyIO32CmdBuff;
    unsigned char *pbyIO32Ret = abyIO32RetBuff;
    const bool bDoACK = true;
    const bool bReadACK = false;

    nIO32Cmd += addStart(pbyIO32Cmd + nIO32Cmd);
    nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, byI2CAddress << 1, bReadACK);

    for ( int n = 0; n < byNumBytes; ++n )
    {
      nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, byFirstRegAddress + n, bReadACK);
      nIO32Cmd += add(pbyIO32Cmd + nIO32Cmd, pbySrcData[n], bReadACK);
    }

    nIO32Cmd += addStop(pbyIO32Cmd + nIO32Cmd);

    m_pIO32->DoIO(nIO32Cmd, pbyIO32Cmd, pbyIO32Ret, 9 * (1 + 2 * byNumBytes));

    int nRet = 8;
    bool bOK = true;
    bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
    nRet += 9;
    for ( int n = 0; n < byNumBytes; ++n )
    {
      bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
      nRet += 9;
      bOK = bOK && IHAL_IO32::IsReadLevelLow(pbyIO32Ret[nRet]);
      nRet += 9;
    }
    return 0;
  }

private:
  int addStart(unsigned char *pbyIO32Buff)
  {
    int nIO32Cmd = 0;

    pbyIO32Buff[nIO32Cmd++] = m_byBME280DataHigh;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockHigh;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280DataLow;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockLow;

    return nIO32Cmd;
  }
  int add(unsigned char *pbyIO32Buff, unsigned char byData, bool bDoACK)
  {
    int nIO32Cmd = 0;

    for ( int nBit = 0; nBit < 8; nBit++ )
    {
      if ( byData & (1 << (7 - nBit)) )
      {
        pbyIO32Buff[nIO32Cmd++] = m_byBME280DataHigh;
      }
      else
      {
        pbyIO32Buff[nIO32Cmd++] = m_byBME280DataLow;
      }
      pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockHigh;
      pbyIO32Buff[nIO32Cmd++] = m_byBME280DataRead;
      pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockLow;
    }

    if ( bDoACK )
    {
      pbyIO32Buff[nIO32Cmd++] = m_byBME280DataLow;
    }
    else
    {
      pbyIO32Buff[nIO32Cmd++] = m_byBME280DataHigh;
    }
    pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockHigh;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280DataRead;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockLow;

    return nIO32Cmd;
  }
  int addStop(unsigned char *pbyIO32Buff)
  {
    int nIO32Cmd = 0;

    pbyIO32Buff[nIO32Cmd++] = m_byBME280DataLow;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280ClockHigh;
    pbyIO32Buff[nIO32Cmd++] = m_byBME280DataHigh;

    return nIO32Cmd;
  }

private:
  IHAL_IO32 * m_pIO32 = nullptr;
  unsigned char m_byBME280DataRead = 0;
  unsigned char m_byBME280DataHigh = 0;
  unsigned char m_byBME280DataLow = 0;
  unsigned char m_byBME280ClockHigh = 0;
  unsigned char m_byBME280ClockLow = 0;
};

#ifdef C_BME
static IHAL_IO32 *s_pIO32 = nullptr;
#endif
int main()
{
  static const unsigned char sc_byBME280GNDLow = IHAL_IO32::functionLow | IHAL_IO32::pin27;
  static const unsigned char sc_byBME280VccOff = IHAL_IO32::functionIn | IHAL_IO32::pin28;
  static const unsigned char sc_byBME280VccHigh = IHAL_IO32::functionHigh | IHAL_IO32::pin28;

  CWinCommDevice commIO32USB(EWinCommType::WinUSBDevice);
  CHAL_IO32_ICommProxy io32(&commIO32USB);
  CBME280I2CCallbackIO32Impl bme280i2c(&io32, IHAL_IO32::pin25, IHAL_IO32::pin26);
  ICommDevice *pCommIO32 = &commIO32USB;
  pCommIO32->Connect();
#ifdef C_BME
  s_pIO32 = &io32;
#endif
  io32.DoIO(1, &sc_byBME280VccOff, nullptr, 0);

  Sleep(1000);

  io32.DoIO(1, &sc_byBME280GNDLow, nullptr, 0);
  io32.DoIO(1, &sc_byBME280VccHigh, nullptr, 0);

  Sleep(5);

  long lTemp = 0;
  unsigned long dwPress = 0;
  unsigned long dwRelHum = 0;
#ifdef C_BME
#if 0
  SBME280 BME280c;
  BME280_Init(&BME280c, BME280_I2C_ADDRESS_SDO_LOW);
  BME280_TriggerMeasurement(&BME280c);
  while ( BME280_IsMeasurementInProgress(&BME280c) );

  BME280_ReadMeasuredResults(&BME280c, &lTemp, &dwPress, &dwRelHum);
#endif
#endif

  CBME280 BME280;

  BME280.Init(CBME280::sc_byBME280_I2C_ADDRESS_SDO_LOW, &bme280i2c);

  BME280.TriggerMeasurement();
  while ( BME280.IsMeasurementInProgress() );

  //long lTemp = 0;
  //unsigned long dwPress = 0;
  //unsigned long dwRelHum = 0;
  BME280.ReadMeasuredResults(&lTemp, &dwPress, &dwRelHum);

  float fTemp = (float)(lTemp / 100);
  dwPress = (dwPress + 99) / 100;
  dwRelHum = (dwRelHum + 999) / 1000;
  float fRelHum = (float)dwRelHum;

  printf("Temp: %.1FdegC\n\rPress: %dmBar\n\rHum: %.0F%%\n\r", fTemp, dwPress, fRelHum);
  system("pause");
  return 0;
}

#ifdef C_BME
typedef enum _SingleI2CMasterHAL_Command
{
  SingleI2CMasterHAL_ValueMask = 0x00FF,
  SingleI2CMasterHAL_DoSTART = 0x0100,
  SingleI2CMasterHAL_DoACK = 0x0200,
  SingleI2CMasterHAL_DoSTOP = 0x0400,
}SingleI2CMasterHAL_Command;

typedef enum _SingleI2CMasterHAL_Result
{
  SingleI2CMasterHAL_RetValueMask = 0x00FF,
  SingleI2CMasterHAL_ACK = 0x0100,
  SingleI2CMasterHAL_ErrorMask = 0x7000,
  SingleI2CMasterHAL_ErrorTimeout = 0x1000,
}SingleI2CMasterHAL_Result;

typedef unsigned short (fnSingleI2CMasterCmd)(unsigned short wCmd);

#define I2CCmd pfnI2CCmd

unsigned char SingleI2CMasterHAL_StartReadStop(fnSingleI2CMasterCmd *pfnI2CCmd,
  unsigned char byI2CAddress,
  unsigned char byFirstRegAddress,
  unsigned char byNumRegsToRead,
  unsigned char *pbyRegsData)
{
  unsigned short wResponse = 0;
  wResponse = I2CCmd(SingleI2CMasterHAL_DoSTART | (byI2CAddress << 1));
  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    wResponse = I2CCmd(byFirstRegAddress);
  }
  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    wResponse = I2CCmd(SingleI2CMasterHAL_DoSTART | (byI2CAddress << 1) | 1);
  }

  while ( byNumRegsToRead > 1 )
  {
    wResponse = I2CCmd(SingleI2CMasterHAL_DoACK | 0xFF);
    if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
    {
      *pbyRegsData = (unsigned char)wResponse;
    }
    pbyRegsData++;
    byNumRegsToRead--;
  }

  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    wResponse = I2CCmd(SingleI2CMasterHAL_DoSTOP | 0xFF);
  }


  if ( !(wResponse & (SingleI2CMasterHAL_ErrorMask)) )
  {
    *pbyRegsData = (unsigned char)wResponse;
    return 0;
  }

  return ~0;
}

unsigned char SingleI2CMasterHAL_StartWriteStop(fnSingleI2CMasterCmd *pfnI2CCmd,
  unsigned char byI2CAddress,
  unsigned char byFirstRegAddress,
  unsigned char byNumRegsToWrite,
  const unsigned char *pbyRegsData)
{
  unsigned short wResponse = 0;
  wResponse = I2CCmd(SingleI2CMasterHAL_DoSTART | (byI2CAddress << 1));

  while ( byNumRegsToWrite > 1 )
  {
    wResponse = I2CCmd(byFirstRegAddress++);
    if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
    {
      wResponse = I2CCmd(*pbyRegsData++);
    }
  }

  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    wResponse = I2CCmd(byFirstRegAddress);
  }
  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    wResponse = I2CCmd(SingleI2CMasterHAL_DoSTOP | *pbyRegsData);
  }

  if ( !(wResponse & (SingleI2CMasterHAL_ACK | SingleI2CMasterHAL_ErrorMask)) )
  {
    return 0;
  }

  return ~0;
}

void setDATA()
{
  s_pIO32->DoIO(1, &sc_byBME280DataHigh, nullptr, 0);
}
void setCLOCK()
{
  s_pIO32->DoIO(1, &sc_byBME280ClockHigh, nullptr, 0);
}
void clrDATA()
{
  s_pIO32->DoIO(1, &sc_byBME280DataLow, nullptr, 0);
}
void clrCLOCK()
{
  s_pIO32->DoIO(1, &sc_byBME280ClockLow, nullptr, 0);
}
int readDATA()
{
  unsigned char byData = 0;
  s_pIO32->DoIO(1, &sc_byBME280DataRead, &byData, 1);
  if ( IHAL_IO32::IsReadLevelHigh(byData) )
  {
    return 1;
  }
  return 0;
}

void SingleI2CMaster_CmdImpl(unsigned short Command, unsigned short * Result)
{
  unsigned char nBit;
  *Result = 0;
  if ( Command & SingleI2CMasterHAL_DoSTART )
  {
    setDATA();
    setCLOCK();
    clrDATA();
    clrCLOCK();
  }

  for ( nBit = 0; nBit < 8; nBit++ )
  {
    if ( Command & (1 << (7 - nBit)) )
    {
      setDATA();
    }
    else
    {
      clrDATA();
    }

    setCLOCK();

    if ( readDATA() )
    {
      *Result |= (1 << (7 - nBit));
    }

    clrCLOCK();
  }

  if ( Command & SingleI2CMasterHAL_DoACK )
  {
    clrDATA();
  }
  else
  {
    setDATA();
  }

  setCLOCK();

  if ( readDATA() )
  {
    *Result |= SingleI2CMasterHAL_ACK;
  }

  clrCLOCK();

  if ( Command & SingleI2CMasterHAL_DoSTOP )
  {
    clrDATA();
    setCLOCK();
    setDATA();
  }
}

unsigned short i2cCmd(unsigned short wCmd)
{
  unsigned short wResult = 0;
  SingleI2CMaster_CmdImpl(wCmd, &wResult);
  return wResult;
}

unsigned char BME280_I2CStartReadStop(const SBME280 *pBME280, unsigned char byAddress, unsigned char *pbyDestData, unsigned char byNumBytes)
{
  return SingleI2CMasterHAL_StartReadStop(i2cCmd, pBME280->m_byI2CAddress, byAddress, byNumBytes, pbyDestData);
}
unsigned char BME280_I2CStartWriteStop(const SBME280 *pBME280, unsigned char byAddress, const unsigned char *pbySrcData, unsigned char byNumBytes)
{
  return SingleI2CMasterHAL_StartWriteStop(i2cCmd, pBME280->m_byI2CAddress, byAddress, byNumBytes, pbySrcData);
}
#endif
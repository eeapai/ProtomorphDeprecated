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

#include "Log.h"
#include <stdio.h>
#include <stdarg.h>

#include <ostream>

#ifndef MAX_LOG_LINE_LENGTH
#define MAX_LOG_LINE_LENGTH 256
#endif // !MAX_LOG_LINE_LENGTH


CLog * CLog::ms_pOnlyInstance = nullptr;
CLog CLog::ms_OnlyInstance;

CLog::ILog *CLog::ms_pLog = nullptr;
CLog::fnOut *CLog::ms_pfnOut = nullptr;
const char *CLog::ms_pcszNewLine = "\n\r";

void CLog::Log(int nFlags, const char * pcszFormat, ...)
{
  char achBuffer[MAX_LOG_LINE_LENGTH];
  int nStratOffset = 2;
  int nNewLineLenght = (int)strlen(ms_pcszNewLine);
  int nAvailable = sizeof(achBuffer) - nStratOffset - nNewLineLenght;
  int nResult;
  va_list args;
  va_start(args, pcszFormat);
  nResult = vsnprintf(achBuffer + nStratOffset, nAvailable, pcszFormat, args);
  va_end(args);

  int nWritten = nResult;
  if ( nResult >= nAvailable )
  {
    nWritten = nAvailable - 1;
  }
  int nNullPos = nStratOffset + nWritten;
  achBuffer[nNullPos] = 0;

  if ( nResult < 0 )
  {
    return;
  }

  achBuffer[0] = ' ';
  achBuffer[1] = ' ';
  const char *pcszPrefix = "  ";
  switch ( nFlags & CLog::sc_nSeverityMask )
  {
  case CLog::sc_nSeverityError  : achBuffer[0] = 'E'; break;
  case CLog::sc_nSeverityWaring : achBuffer[0] = 'W'; break;
  case CLog::sc_nSeverityInfo   : achBuffer[0] = 'I'; break;
  case CLog::sc_nSeverityDebug  : achBuffer[0] = 'D'; break;
  default:
    break;
  }

  if ( nFlags & CLog::sc_nFormatNewLine )
  {
    memmove(&achBuffer[nNullPos], ms_pcszNewLine, nNewLineLenght);
    achBuffer[nNullPos + nNewLineLenght] = 0;
  }

  if ( ms_pLog )
  {
    ms_pLog->Out(achBuffer);
  }

  if ( ms_pfnOut )
  {
    ms_pfnOut(achBuffer);
  }
}

void CLog::SetEnv(const char * pcszNewLine, ILog *pLog)
{
  ms_pcszNewLine = pcszNewLine;
  ms_pLog = pLog;
}

void CLog::SetEnv(const char * pcszNewLine, fnOut * pfnOut)
{
  ms_pcszNewLine = pcszNewLine;
  ms_pfnOut = pfnOut;
}

void CLogSource::doLog(int nFlags, const char * pcszFormat, ...)
{
  char achBuffer[MAX_LOG_LINE_LENGTH];
  int nResult;
  va_list args;
  va_start(args, pcszFormat);
  nResult = vsnprintf(achBuffer, sizeof(achBuffer), pcszFormat, args);
  va_end(args);

  if ( nResult < 0 )
  {
    CLog::Log(CLog::sc_nFormatNewLine | CLog::sc_nSeverityError, "CAN'T FORMAT LOG");
    return;
  }

  int nWritten = nResult;
  if ( nResult >= sizeof(achBuffer) )
  {
    nWritten = sizeof(achBuffer) - 1;
  }
  achBuffer[nWritten] = 0;

  int nLogFlags = nFlags | CLog::sc_nFormatNewLine;

  if ( m_pcszSource )
  {
    CLog::Log(nLogFlags, "%s: %s", m_pcszSource, achBuffer);
    return;
  }
  if ( m_pcszFormat && m_pcszParam )
  {
    CLog::Log(nLogFlags, "%s[%s]: %s", m_pcszFormat, m_pcszParam, achBuffer);
    return;
  }
  if ( m_pcszFormat )
  {
    CLog::Log(nLogFlags, "%s[%d]: %s", m_pcszFormat, m_nParam, achBuffer);
    return;
  }
  CLog::Log(nLogFlags, "%s", achBuffer);
}

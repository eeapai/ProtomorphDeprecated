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
#ifndef __LOG_H__
#define __LOG_H__

class CLog
{
public:
  CLog() { ms_pOnlyInstance = this; }
  virtual ~CLog(){}
  struct ILog
  {
    virtual ~ILog() {}
    virtual void Out(const char *pcszString) = 0;
  };
private:
  static CLog *ms_pOnlyInstance;
  static CLog ms_OnlyInstance;

public:
  static void Log(int nFlags, const char *pcszFormat, ...);

  static const int sc_nSeverityMask   = 0x00000003;
  static const int sc_nSeverityError  = 0x00000000;
  static const int sc_nSeverityWaring = 0x00000001;
  static const int sc_nSeverityInfo   = 0x00000002;
  static const int sc_nSeverityDebug  = 0x00000003;
  static const int sc_nFormatNewLine  = 0x80000000;

  static void SetEnv(const char *pcszNewLine, ILog *pLog);

  typedef void(fnOut)(const char *pcszString);

  static void SetEnv(const char *pcszNewLine, fnOut *pfnOut);

private:
  static ILog *ms_pLog;
  static fnOut *ms_pfnOut;
  static const char *ms_pcszNewLine;
};

class CLogSource
{
public:
  CLogSource(const char *pcszSource) : m_pcszSource(pcszSource) { }
  CLogSource(const char *pcszFormat, int nParam) : m_pcszFormat(pcszFormat), m_nParam(nParam) { }
  CLogSource(const char *pcszFormat, const char *pcszParam) : m_pcszFormat(pcszFormat), m_pcszParam(pcszParam) { }



  template<class... Args>
  void Error(const char *pcszFormat, Args&&... args)
  {
    doLog(CLog::sc_nSeverityError, pcszFormat, std::forward<Args>(args)...);
  }
  template<class... Args>
  void Warning(const char *pcszFormat, Args&&... args)
  {
    doLog(CLog::sc_nSeverityWaring, pcszFormat, std::forward<Args>(args)...);
  }
  template<class... Args>
  void Info(const char *pcszFormat, Args&&... args)
  {
    doLog(CLog::sc_nSeverityInfo, pcszFormat, std::forward<Args>(args)...);
  }
  template<class... Args>
  void Debug(const char *pcszFormat, Args&&... args)
  {
    doLog(CLog::sc_nSeverityDebug, pcszFormat, std::forward<Args>(args)...);
  }

private:
  void doLog(int nFlags, const char *pcszFormat, ...);

private:
  const char *m_pcszSource = nullptr;
  const char *m_pcszFormat = nullptr;
  int m_nParam = 0;
  const char *m_pcszParam = nullptr;

};

#endif
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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <string>
inline std::wstring a2strw(const CHAR *pcszSrc)
{
  int size_needed = MultiByteToWideChar(CP_UTF8, 0, pcszSrc, -1, NULL, 0);
  WCHAR *pDest = new WCHAR[size_needed];
  MultiByteToWideChar(CP_UTF8, 0, pcszSrc, -1, (LPWSTR)pDest, size_needed);
  std::wstring str = pDest;
  delete[] pDest;
  return str;
}

inline std::string w2str(const WCHAR *pcszSrc)
{
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, pcszSrc, -1, NULL, 0, NULL, NULL);
  CHAR *pDest = new CHAR[size_needed];
  WideCharToMultiByte(CP_UTF8, 0, pcszSrc, -1, (LPSTR)pDest, size_needed, NULL, NULL);
  std::string str = pDest;
  delete[] pDest;
  return str;
}

#define A2CSTRW(str) a2strw(str).c_str()
#define W2STR(wstr) w2str(wstr).c_str()

#ifdef UNICODE
#define _TPARAM(mbcs) A2CSTRW(mbcs)
#else
#define _TPARAM(mbcs) (mbcs)
#endif

#endif

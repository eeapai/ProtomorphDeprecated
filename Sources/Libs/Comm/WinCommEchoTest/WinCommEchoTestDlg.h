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

// WinCommEchoTestDlg.h : header file
//

#pragma once

#include "ICommDevice.h"
#include "WinCommFactory.h"

// CWinCommEchoTestDlg dialog
class CWinCommEchoTestDlg : public CDialogEx
{
  typedef CDialogEx base_class;
// Construction
public:
	CWinCommEchoTestDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CWinCommEchoTestDlg();

  struct STestConfiguration
  {
    DWORD m_dwConstatPacketSize = 8 * 1024;
    BOOL m_bRunContiniously = FALSE;
  };
  STestConfiguration m_sTestConfiguration;
  struct STestResult
  {
    DWORD m_dwSpeedInBps = 0;
  };
  STestResult m_sTestResult;
// Dialog Data
	enum { IDD = IDD_COMMTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  void UpdateControls();


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedCancel();
  afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);

private:
  void enumerateDevices();
  void doOneWinCommEchoTest();

  void connect();
  void disconnect();
private:
  BOOL m_bRunning = FALSE;
  CString m_strDeviceName;
  CComboBox	m_wndDeviceName;
  
  DWORD m_dwAvailablePacketSize = 0;
  BYTE *m_pbyTestBuffer = nullptr;

  CWinCommDevice m_commDevice;
  
public:
  afx_msg void OnBnClickedDoTest();
  afx_msg void OnBnClickedStopTest();
  afx_msg void OnCbnDropdownComboName();
};
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

// WinCommEchoTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinCommEchoTest.h"
#include "WinCommEchoTestDlg.h"
#include "afxdialogex.h"

#include "WinCommFactory.h"
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWinCommEchoTestDlg dialog




CWinCommEchoTestDlg::CWinCommEchoTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWinCommEchoTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CWinCommEchoTestDlg::~CWinCommEchoTestDlg()
{
  if ( m_pbyTestBuffer )
  {
    delete[] m_pbyTestBuffer;
    m_pbyTestBuffer = NULL;
  }
}

void CWinCommEchoTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_TYPE, m_wndDeviceType);
  DDX_Control(pDX, IDC_COMBO_NAME, m_wndDeviceName);
  DDX_CBIndex(pDX, IDC_COMBO_TYPE, m_nDeviceType);
  DDX_CBString(pDX, IDC_COMBO_NAME, m_strDeviceName);
  DDX_Check(pDX, IDC_CHECK1, m_sTestConfiguration.m_bRunContiniously);

  DDX_Text(pDX, IDC_EDIT4, m_sTestConfiguration.m_dwConstatPacketSize);

  if ( !pDX->m_bSaveAndValidate )
  {
    DDX_Text(pDX, IDC_EDIT6, m_sTestResult.m_dwSpeedInBps);
  }
}

void CWinCommEchoTestDlg::UpdateControls()
{
  UpdateData(TRUE);
  UpdateData(FALSE);

  CWnd * pCtrl = NULL;
  pCtrl = GetDlgItem(IDC_COMBO_NAME); pCtrl->EnableWindow(!m_bRunning);
  pCtrl = GetDlgItem(IDC_EDIT4); pCtrl->EnableWindow(!m_bRunning);
  pCtrl = GetDlgItem(IDC_CHECK1); pCtrl->EnableWindow(!m_bRunning);
  pCtrl = GetDlgItem(IDC_BUTTON1); pCtrl->EnableWindow(!m_bRunning);
  pCtrl = GetDlgItem(IDC_BUTTON2); pCtrl->EnableWindow(m_bRunning);
}


BEGIN_MESSAGE_MAP(CWinCommEchoTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_BN_CLICKED(IDCANCEL, &CWinCommEchoTestDlg::OnBnClickedCancel)
  ON_BN_CLICKED(IDC_BUTTON1, &CWinCommEchoTestDlg::OnBnClickedDoTest)
  ON_BN_CLICKED(IDC_BUTTON2, &CWinCommEchoTestDlg::OnBnClickedStopTest)
  ON_CBN_DROPDOWN(IDC_COMBO_NAME, &CWinCommEchoTestDlg::OnCbnDropdownComboName)
END_MESSAGE_MAP()


// CWinCommEchoTestDlg message handlers

BOOL CWinCommEchoTestDlg::OnInitDialog()
{
  USES_CONVERSION;

	CDialogEx::OnInitDialog();
  
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  m_wndDeviceType.AddString(A2CT("WinUSB"));
  m_wndDeviceType.AddString(A2CT("WinSock"));

	// TODO: Add extra initialization here
  enumerateDevices();
  UpdateData(FALSE);
  UpdateControls();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

EWinCommType CWinCommEchoTestDlg::getCommType() const
{
  EWinCommType eComm = EWinCommType::WinUSBDevice;
  switch ( m_nDeviceType )
  {
  case devtypWinUSB: eComm = EWinCommType::WinUSBDevice; break;
  case devtypWinSock: eComm = EWinCommType::WinSockWrapper; break;
  default:
    break;
  }
  return eComm;
}

void CWinCommEchoTestDlg::enumerateDevices()
{
  USES_CONVERSION;

  EWinCommType eComm = getCommType();

  char achDevice[1024] = { 0 };
  std::vector<std::string> devs;
  while ( 0 < CWinCommDevice::ListConnection(eComm, (int)devs.size(), achDevice, sizeof(achDevice) -1) )
  {
    std::string strDev = achDevice;
    devs.push_back(strDev);
    memset(achDevice, 0, sizeof(achDevice));
  }
  m_wndDeviceName.ResetContent();
  for ( DWORD I = 0; I < devs.size(); I++ )
  {
    m_wndDeviceName.AddString(A2CT(devs[I].c_str()));
  }

  if ( ( devs.size() > 0 ) && m_strDeviceName.IsEmpty() )
  {
    m_strDeviceName = devs[0].c_str();
  }
}

void CWinCommEchoTestDlg::doOneWinCommEchoTest()
{
  bool bResult = true;
  m_sTestResult.m_dwSpeedInBps = 0;
  DWORD dwTestSize = m_sTestConfiguration.m_dwConstatPacketSize;

  BYTE bySeed = (BYTE)rand();

  for ( DWORD I = 0; I < dwTestSize; I++ )
  {
    m_pbyTestBuffer[I] = (BYTE)(bySeed + I);
  }

  LARGE_INTEGER countStart;
  QueryPerformanceCounter(&countStart);

  DWORD dwSent = 0;
  while ( bResult && ( dwSent < dwTestSize ) )
  {
    DWORD dwJustSent = m_commDevice.Send(m_pbyTestBuffer + dwSent, dwTestSize - dwSent);
    if ( ICommDevice::connectionConnected != m_commDevice.GetStatus() )
    {
      bResult = false;
    }
    dwSent += dwJustSent;
  }
  
  memset(m_pbyTestBuffer, 0, dwTestSize);

  DWORD dwReceived = 0;
  while ( bResult && ( dwReceived < dwTestSize ) )
  {
    DWORD dwJustReceived = m_commDevice.Receive(m_pbyTestBuffer + dwReceived, dwTestSize - dwReceived);
    if ( ICommDevice::connectionConnected != m_commDevice.GetStatus() )
    {
      bResult = false;
    }
    dwReceived += dwJustReceived;
  }

  LARGE_INTEGER countEnd;
  QueryPerformanceCounter(&countEnd);
  for ( DWORD I = 0; I < dwTestSize; I++ )
  {
    if ( m_pbyTestBuffer[I] != (BYTE)(bySeed + I) )
    {
      bResult = false;
      break;
    }
  }

  if ( !bResult )
  {
    OnBnClickedStopTest();
    return;
  }

  LARGE_INTEGER f;
  QueryPerformanceFrequency( &f );

  LARGE_INTEGER elapsedCount;
  elapsedCount.QuadPart = countEnd.QuadPart - countStart.QuadPart;

  DOUBLE elapsed = (DOUBLE)elapsedCount.QuadPart / (DOUBLE)f.QuadPart;
  m_sTestResult.m_dwSpeedInBps = (DWORD)((dwTestSize << 1) / elapsed);
}

void CWinCommEchoTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinCommEchoTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinCommEchoTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CWinCommEchoTestDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
  if ( !m_bRunning )
  {
    return TRUE;
  }
  doOneWinCommEchoTest();
  UpdateData(FALSE);
  if ( !m_sTestConfiguration.m_bRunContiniously )
  {
    m_bRunning = FALSE;
    disconnect();
    UpdateControls();
  }
  return TRUE;
}

void CWinCommEchoTestDlg::OnBnClickedCancel()
{
  // TODO: Add your control notification handler code here
  CDialogEx::OnCancel();
}


void CWinCommEchoTestDlg::OnBnClickedDoTest()
{
  UpdateData(TRUE);
  connect();
  BOOL bResult = TRUE;
  
  if ( bResult )
  {
    if ( m_sTestConfiguration.m_dwConstatPacketSize > m_dwAvailablePacketSize )
    {
      if ( m_pbyTestBuffer )
      {
        delete[] m_pbyTestBuffer;
        m_pbyTestBuffer = NULL;
        m_dwAvailablePacketSize = 0;
      }
      m_pbyTestBuffer = new BYTE[m_sTestConfiguration.m_dwConstatPacketSize];
      bResult = NULL != m_pbyTestBuffer;
      m_dwAvailablePacketSize = m_sTestConfiguration.m_dwConstatPacketSize;
    }
  }

  m_bRunning = bResult;
  UpdateData(FALSE);
  UpdateControls();
}

void CWinCommEchoTestDlg::connect()
{
  m_commDevice.SetType(getCommType());
  CT2A deviceName(m_strDeviceName, CP_UTF8);
  m_commDevice.Connect(deviceName.m_psz);
}

void CWinCommEchoTestDlg::disconnect()
{
  m_commDevice.Disconnect();
}


void CWinCommEchoTestDlg::OnBnClickedStopTest()
{
  m_bRunning = FALSE;
  disconnect();
  UpdateData(FALSE);
  UpdateControls();
}


void CWinCommEchoTestDlg::OnCbnDropdownComboName()
{
  UpdateData(TRUE);
  enumerateDevices();
  UpdateData(FALSE);
}
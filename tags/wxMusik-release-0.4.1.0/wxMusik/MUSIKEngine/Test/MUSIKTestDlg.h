//Copyright (c) 2004, Gunnar Roth
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided 
//that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and 
//      the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
//      and the following disclaimer in the documentation and/or other materials provided with the distribution.
//    * Neither the name of the Gunnar Roth nor the names of its contributors may be used to endorse or 
//      promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// MUSIKTestDlg.h : Headerdatei
//

#pragma once
#include "afxwin.h"
#include "FMODEngine/inc/fmodengine.h"
#include "MUSIKEngine/inc/stream.h"
#include "afxcmn.h"

// CMUSIKTestDlg Dialogfeld
class CMUSIKTestDlg : public CDialog
{
// Konstruktion
public:
	CMUSIKTestDlg(CWnd* pParent = NULL);	// Standardkonstruktor

	~CMUSIKTestDlg()
	{
		delete m_pStream;
	}
// Dialogfelddaten
	enum { IDD = IDD_MUSIKTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;

	bool m_Seeking;
	FMODEngine m_FMODEng;

	MUSIKStream * m_pStream;
	bool OpenFileDlg();
	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void  OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	CStatic m_stTimeCap;
	afx_msg void OnBnClickedCancel();
	CSliderCtrl m_SeekSlider;
};

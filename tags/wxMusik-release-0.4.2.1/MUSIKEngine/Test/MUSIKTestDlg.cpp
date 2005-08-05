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

// MUSIKTestDlg.cpp : Implementierungsdatei
//

#include "stdafx.h"

#include "MUSIKTest.h"
#include "MUSIKTestDlg.h"
#include ".\MUSIKtestdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg-Dialogfeld für Anwendungsbefehl 'Info'

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialogfelddaten
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

// Implementierung
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CMUSIKTestDlg Dialogfeld



CMUSIKTestDlg::CMUSIKTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMUSIKTestDlg::IDD, pParent)
	,m_FMODEng(44100,32,0x0002)
{
	m_pStream = NULL;
	m_Seeking = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMUSIKTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIMECAP, m_stTimeCap);
	DDX_Control(pDX, IDC_SLIDER_TIME, m_SeekSlider);
}

BEGIN_MESSAGE_MAP(CMUSIKTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CMUSIKTestDlg Meldungshandler

BOOL CMUSIKTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Hinzufügen des Menübefehls "Info..." zum Systemmenü.

	// IDM_ABOUTBOX muss sich im Bereich der Systembefehle befinden.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	// TODO: Hier zusätzliche Initialisierung einfügen
	
	return TRUE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}

void CMUSIKTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CMUSIKTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CMUSIKTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMUSIKTestDlg::OnBnClickedButtonOpen()
{
	if(OpenFileDlg())
		m_pStream->Play();
}
#define SEEK_SCALE (6*1000)

bool CMUSIKTestDlg::OpenFileDlg()
{

	CFileDialog dlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST,"All media|*.ape;*.flac;*.wmv;*.wma;*.mp3;*.ogg;*.wav;*.mpc;*.mp+;*.mpp|All files|*.*||",this);
	if(dlg.DoModal()==IDCANCEL)
		return false;


	if(m_pStream)
		delete m_pStream;
	m_pStream = m_FMODEng.OpenMedia(dlg.GetFileName());
	SetTimer( 1, 333, 0);
	m_SeekSlider.SetRange(0,m_pStream->GetLengthMs()/SEEK_SCALE,true);
	return true;
}
void CMUSIKTestDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CMUSIKTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(m_pStream)
	{
		int timeMS = m_Seeking ? m_SeekSlider.GetPos()*SEEK_SCALE:m_pStream->GetTime();
		
		CString s;
		MUSIKDecoder::INFO * pStrInfo = m_pStream->GetDecoderInfo();
		s.Format("Length %02d:%02d Time %02d:%02d\nBitRate: %d kbps\nSamplerate: %d Hz\nChannels: %d\nBits per sample: %d\nType: %s\nCompression: %d%%", 
			m_pStream->GetLengthMs() / 1000 / 60, m_pStream->GetLengthMs() / 1000 % 60, 
			timeMS / 1000 / 60, timeMS / 1000 % 60,pStrInfo->bitrate, 
			pStrInfo->frequency,pStrInfo->channels, pStrInfo->bits_per_sample,m_pStream->Type(),(int)(0.5f + 100.f-((pStrInfo->bitrate / 1411.0f)*100.f))
			);
		m_stTimeCap.SetWindowText(s);
		if(!m_Seeking)
		{
			int timeMS = m_pStream->GetTime();
			TRACE( "Slider SetPos= %d\n", timeMS );
			m_SeekSlider.SetPos(timeMS/ SEEK_SCALE);
		}
		
	}
}
void  CMUSIKTestDlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{

	if( nSBCode == TB_THUMBTRACK )
		m_Seeking = true;
	if( nSBCode == TB_ENDTRACK || nSBCode == TB_THUMBPOSITION)
	{
		CSliderCtrl *slider = (CSliderCtrl*)pScrollBar;
		if(slider && m_pStream)
		{
			int timeMS = slider->GetPos()*SEEK_SCALE;
			m_pStream->SetTime(timeMS);
			TRACE( "Slider GetPos= %d\n", timeMS );

			m_Seeking = false;
		}

	}

}
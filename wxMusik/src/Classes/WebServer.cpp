/*
 *  WebServer.cpp
 *
 *  Musik's internal webserver. 
 *  Someday this will be quite the little jewel.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "WebServer.h"
#include "../Frames/MusikFrame.h"

//--- globals ---//
#include "../MusikUtils.h"
#include "../MusikGlobals.h"

CMusikWebServer::CMusikWebServer(CMusikPlayer * pPlayer)
:m_pPlayer(pPlayer)
{
	m_bRunning = false;
}

CMusikWebServer::~CMusikWebServer()
{
}

void CMusikWebServer::Start()
{
	wxIPV4address addr;
    addr.Service( wxGetApp().Prefs.nWebServerPort );

	pServer = new wxSocketServer( addr );

	pServer->SetEventHandler( *g_MusikFrame, MUSIK_SERVER_ID );
	pServer->SetNotify( wxSOCKET_CONNECTION_FLAG );
	pServer->Notify( TRUE );

	m_bRunning = true;
	m_bListenLock = false;
}

void CMusikWebServer::Stop()
{
	if ( m_bRunning )
	{
		pServer->Destroy();

		m_bRunning = false;
		m_bListenLock = false;
	}
}

// this gets called from the main frame event handler, listens for a connection
// if there is one it processes the request, if not, exits
// it should get called only when there is a pending connection, but to make sure
// things synch up we use the blocking Accept (non-blocking sometimes wouldn't catch connections)
// TODO: thread this
void CMusikWebServer::Listen()
{
	if ( m_bListenLock )
		return;

	if ( !m_bRunning )
		return;

	m_bListenLock = true;

	pSocket = pServer->Accept( true );
	if ( !pSocket )
	{
		return;
	}

	// got a client, let's continue
	pServer->SetTimeout( 30 );

	bool close = false;
	while ( !close )
	{
		wxString s;
		//wxChar ch = wxT('\0');
	    
		wxString instr;
		if ( ReadLine( instr ) > 2 )
		{
			ProcessRequest( instr );
		}
		else
		{
			close = true;
		}
	    
	}
	
	pSocket->Destroy();

	m_bListenLock = false;
}

// looks at the request string and executes the right command
// also outputs a result to the browser
// TODO: output something other than just echoing the request
void CMusikWebServer::ProcessRequest(wxString reqstr)
{
	if ( reqstr.Left( 3 ) == wxT("GET") )
	{
		if ( reqstr.Find( wxT("next.mkc") ) > -1 )
		{
			m_pPlayer->NextSong();
		}

		if ( reqstr.Find( wxT("previous.mkc") ) > -1 )
		{
			m_pPlayer->PrevSong();
		}

		if ( reqstr.Find( wxT("pause.mkc") ) > -1 )
		{
			m_pPlayer->Pause();
		}

		if ( reqstr.Find( wxT("stop.mkc") ) > -1 )
		{
			m_pPlayer->Stop();
		}

		if ( reqstr.Find( wxT("play.mkc") ) > -1 )
		{
			m_pPlayer->PlayPause();
		}
		
		if ( reqstr.Find( wxT("resume.mkc" ) ) > -1 )
		{
			m_pPlayer->Resume();
		}

		wxString server_version;
		wxString server_title;

		server_version.sprintf( wxT( "Server: %s\r\n" ), MUSIKSERV_VERSION );
		server_title.sprintf( wxT( "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\r\n" ), MUSIKAPPNAME_VERSION );

		WriteLine( wxT("HTTP/1.1 200 OK\r\n") );
		WriteLine( server_version );
		WriteLine( wxT("Content-Type: text/html; charset=utf8\r\n") );
		WriteLine( wxT("\r\n") );
		WriteLine( server_title );
		WriteLine( wxT("<body>\r\n") );

		WriteLine( reqstr );

		WriteLine( wxT("</body>\r\n") );
		WriteLine( wxT("</html>\r\n") );	
	}	
}

// reads a line of input from the socket
int CMusikWebServer::ReadLine(wxString& outstr)
{
	int n;
	for ( n = 1; n < MAX_BUF_LEN; n++ )
	{
		char c;
		pSocket->Read( &c, 1 );
		int charsread = pSocket->LastCount();

		if ( charsread == 1 )
		{
			outstr += c;
			if ( c == '\n' )
				break;
		}
		else if ( charsread == 0 )
		{
			if ( n == 1 )
				return 0;
			else
				break;
		}
    }

	return n;
	
}

// writes a line of output to the socket
void CMusikWebServer::WriteLine( wxString str )
{
    pSocket->Write( ( const char* )ConvToUTF8( str ), str.Length() );
}

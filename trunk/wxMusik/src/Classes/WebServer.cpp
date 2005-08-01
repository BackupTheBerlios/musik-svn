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
#include "myprec.h"

#include "WebServer.h"
#include "../Frames/MusikFrame.h"

//--- globals ---//
#include "../MusikUtils.h"
#include "../MusikGlobals.h"

enum
{
  // id for sockets
  SERVER_ID = 100,
  SOCKET_ID
};

BEGIN_EVENT_TABLE(CMusikWebServer, wxEvtHandler)
  EVT_SOCKET(SERVER_ID,  CMusikWebServer::OnServerEvent)
  EVT_SOCKET(SOCKET_ID,  CMusikWebServer::OnSocketEvent)
END_EVENT_TABLE()

CMusikWebServer::CMusikWebServer(CMusikPlayer * pPlayer)
:m_pPlayer(pPlayer)
{
	m_bRunning = false;
    m_nPort = 0;
}

CMusikWebServer::~CMusikWebServer()
{
    Stop();
}

void CMusikWebServer::Start( int nPort)
{
    if(m_bRunning && nPort == m_nPort)
        return;
    else if(m_bRunning)
        Stop();
    m_nPort = nPort;
	wxIPV4address addr;
    addr.Service( m_nPort );

	pServer = new wxSocketServer( addr );

	pServer->SetEventHandler( *this, SERVER_ID );
	pServer->SetNotify( wxSOCKET_CONNECTION_FLAG );
	pServer->Notify( TRUE );

	m_bRunning = true;
}

void CMusikWebServer::Stop()
{
	if ( m_bRunning )
	{
		pServer->Destroy();

		m_bRunning = false;
	}
}


// things synch up we use the blocking Accept (non-blocking sometimes wouldn't catch connections)
// TODO: thread this
void CMusikWebServer::OnServerEvent(wxSocketEvent& event)
{
	if ( !m_bRunning )
		return;

	//------------------------------------------------------//
	//--- if we get a connection, make the server listen ---//
	//------------------------------------------------------//
	switch( event.GetSocketEvent() )
	{
		case wxSOCKET_CONNECTION:
			break;
		default:
			return;
	}

	wxSocketBase *pSocket = pServer->Accept( true );
	if ( !pSocket )
	{
		return;
	}
        pSocket->SetFlags(wxSOCKET_BLOCK);// block gui, fix reentrancy problem.
	pSocket->SetEventHandler(*this, SOCKET_ID);
	pSocket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	pSocket->Notify(true);
}
void CMusikWebServer::OnSocketEvent(wxSocketEvent& event)
{
  wxSocketBase *sock = event.GetSocket();

 
  // Now we process the event
  switch(event.GetSocketEvent())
  {
    case wxSOCKET_INPUT:
    {
      // We disable input events, so that the test doesn't trigger
      // wxSocketEvent again. this prohibits reentrance
      sock->SetNotify(wxSOCKET_LOST_FLAG);

	for ( ;; )
	{
		wxString instr;
		if ( ReadLine(sock, instr ) > 2 )
		{
			if(ProcessRequest(sock, instr ))
			{
                            sock->Destroy();// request was processed, so close connection.
                            return;
			}
		}
		else
		{
			break;
		}
	}
	
      // Enable input events again.
      sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
      break;
    }
    case wxSOCKET_LOST:
    {
      // Destroy() should be used instead of delete wherever possible,
      // due to the fact that wxSocket uses 'delayed events' (see the
      // documentation for wxPostEvent) and we don't want an event to
      // arrive to the event handler (the frame, here) after the socket
      // has been deleted. Also, we might be doing some other thing with
      // the socket at the same time; for example, we might be in the
      // middle of a test or something. Destroy() takes care of all
      // this for us.
      sock->Destroy();
      break;
    }
    default: ;
  }

}
// looks at the request string and executes the right command
// also outputs a result to the browser
// TODO: output something other than just echoing the request
bool CMusikWebServer::ProcessRequest(wxSocketBase*pSocket,const wxString &reqstr)
{

	if ( reqstr.Left( 3 ) == wxT("GET") )
	{
        wxString sRest;
        wxString sCommand = reqstr.AfterFirst('?');
		if ( sCommand.Find( wxT("next.mkc") ) > -1 )
		{
			m_pPlayer->NextSong();
		}

		else if ( sCommand.Find( wxT("previous.mkc") ) > -1 )
		{
			m_pPlayer->PrevSong();
		}

		else if ( sCommand.Find( wxT("pause.mkc") ) > -1 )
		{
			m_pPlayer->Pause();
		}

		else if ( sCommand.Find( wxT("stop.mkc") ) > -1 )
		{
			m_pPlayer->Stop();
		}

		else if ( sCommand.Find( wxT("play.mkc") ) > -1 )
		{
			m_pPlayer->PlayPause();
		}
		
		else if ( sCommand.Find( wxT("resume.mkc" ) ) > -1 )
		{
			m_pPlayer->Resume();
		}
        else if ( sCommand.Find( wxT("volup.mkc" ) ) > -1 )
        {
            m_pPlayer->SetVolume(wxGetApp().Prefs.nSndVolume + 5);
        }
        else if ( sCommand.Find( wxT("voldown.mkc" ) ) > -1 )
        {
            int newvol = wxMax(0,wxGetApp().Prefs.nSndVolume - 5);
            m_pPlayer->SetVolume( newvol );
        } 
        else if( sCommand.StartsWith( wxT("rating=" ) ,&sRest))
        {
            wxGetApp().Library.SetRating(m_pPlayer->GetCurrentSongid(),wxStringToInt(sRest));    
        }
        if(!sCommand.IsEmpty())
        {
            WriteLine(pSocket, wxT("HTTP/1.1 307 Temporary Redirect\r\n") );
            wxString server_version;
            server_version.sprintf( wxT( "Server: %s\r\n" ), MUSIKSERV_VERSION );
            WriteLine(pSocket, server_version );
            WriteLine(pSocket, wxT("Location: /\r\n"));
            WriteLine(pSocket, wxT("\r\n") );

            return true;
        }
        WriteLine(pSocket, wxT("HTTP/1.1 200 OK\r\n") );
        wxString server_version;
        server_version.sprintf( wxT( "Server: %s\r\n" ), MUSIKSERV_VERSION );
        WriteLine(pSocket, server_version );
        WriteLine(pSocket, wxT("Content-Type: text/html; charset=UTF8\r\n") );
        WriteLine(pSocket, wxT("Pragma: no-cache\r\n") );
        WriteLine(pSocket, wxT("\r\n") );

        bool bIsPlaying = m_pPlayer->IsPlaying();
        bool bIsPaused = m_pPlayer->IsPaused();
        std::auto_ptr<CMusikSong> pSong = m_pPlayer->GetCurrentSongid().Song();

        WriteLine(pSocket, wxT( "<HTML><HEAD>\r\n") );
        WriteLine(pSocket, wxString(wxT( "<TITLE>")) << MUSIKAPPNAME_VERSION << wxT("</TITLE>\r\n" ) );
        if(bIsPlaying)
        {
            WriteLine(pSocket,wxString(wxT("<meta http-equiv=\"Refresh\" content=\"")) 
                << wxGetApp().Prefs.nWebServerRefresh
                << wxT("\">\r\n"));
        }
        WriteLine(pSocket,wxT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF8\">"));
		WriteLine(pSocket, wxT("</HEAD><body>\r\n") );

        WriteLine(pSocket,wxT("<table border=\"0\" cellspacing=\"10\" cellpadding=\"2\" >\r\n"));
        WriteLine(pSocket,wxT("<tr>"));
        WriteLine(pSocket,wxT("<td>"));

        WriteLine(pSocket,wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));

        WriteLine(pSocket,wxT("<tr><td>\r\n"));
        WriteLine(pSocket, wxString(_("Title"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Title):wxString(_( "&lt;unknown&gt;"))));
        WriteLine(pSocket,wxT("</td></tr>\r\n"));

        WriteLine(pSocket,wxT("<tr><td>\r\n"));
        WriteLine(pSocket, wxString(_("Artist"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Artist): wxString(_( "&lt;unknown&gt;"))));
        WriteLine(pSocket,wxT("</td></tr>\r\n"));
        WriteLine(pSocket,wxT("<tr><td>\r\n"));
        WriteLine(pSocket, wxString(_("Album"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Album): wxString(_( "&lt;unknown&gt;"))) );
        WriteLine(pSocket,wxT("</td></tr>\r\n"));
        WriteLine(pSocket,wxT("<tr><td>\r\n"));
        WriteLine(pSocket, wxString(_("Rating"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? pSong->Rating : 0) );
        WriteLine(pSocket,wxT("</td></tr>\r\n"));
        WriteLine(pSocket,wxT("<tr><td>\r\n"));
        WriteLine(pSocket, wxString(_("Time"))<< wxT(":</td><td>") << m_pPlayer->GetTimeStr() );
        WriteLine(pSocket,wxT("</td></tr>\r\n"));
        WriteLine(pSocket,wxT("</table>\r\n"));
        WriteLine(pSocket,wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));
        WriteLine(pSocket,wxT("<tr>\r\n"));
        if(!bIsPlaying)
        {
            WriteLine(pSocket,wxT("<td>\r\n"));
            WriteLine(pSocket,wxString(wxT("<a href=\"?play.mkc\" >")) << _("Play") <<wxT("</a>"));
            WriteLine(pSocket,wxT("</td>\r\n"));
        }
        WriteLine(pSocket,wxT("<td>\r\n"));
        if(bIsPaused)
            WriteLine(pSocket,wxString(wxT("<a href=\"?resume.mkc\" >")) << _("Resume") <<wxT("</a>"));
        else
            WriteLine(pSocket,wxString(wxT("<a href=\"?pause.mkc\" >")) << _("Pause") <<wxT("</a>"));
        WriteLine(pSocket,wxT("</td>\r\n"));
        if(bIsPlaying)
        {
            WriteLine(pSocket,wxT("<td>\r\n"));
            WriteLine(pSocket,wxString(wxT("<a href=\"?stop.mkc\" >")) << _("Stop") <<wxT("</a>"));
            WriteLine(pSocket,wxT("</td>\r\n"));
        }
        WriteLine(pSocket,wxT("<td>\r\n"));
        WriteLine(pSocket,wxString(wxT("<a href=\"?previous.mkc\" >")) << wxT("&lt;&lt;") <<wxT("</a>"));
        WriteLine(pSocket,wxT("</td>\r\n"));
        WriteLine(pSocket,wxT("<td>\r\n"));
        WriteLine(pSocket,wxString(wxT("<a href=\"?next.mkc\" >")) << wxT("&gt;&gt;") <<wxT("</a>"));
        WriteLine(pSocket,wxT("</td>\r\n"));

        WriteLine(pSocket,wxT("</tr></table>\r\n"));

        WriteLine(pSocket,wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));
        WriteLine(pSocket,wxT("<tr>\r\n"));
        WriteLine(pSocket,wxT("<td>\r\n"));
        WriteLine(pSocket,wxString(wxT("<a href=\"?volup.mkc\" >")) << wxT("Vol +") <<wxT("</a>"));
        WriteLine(pSocket,wxT("</td>\r\n"));
        WriteLine(pSocket,wxT("<td>\r\n"));
        WriteLine(pSocket,wxString() << wxGetApp().Prefs.nSndVolume);
        WriteLine(pSocket,wxT("</td>\r\n"));
        WriteLine(pSocket,wxT("<td>\r\n"));
        WriteLine(pSocket,wxString(wxT("<a href=\"?voldown.mkc\" >")) << wxT("Vol -") <<wxT("</a>"));
        WriteLine(pSocket,wxT("</td>\r\n"));
        WriteLine(pSocket,wxT("</tr></table>\r\n"));

        WriteLine(pSocket,wxT("</td><td valign=\"top\">"));
        WriteLine(pSocket,wxT("<FORM ACTION=\"/rate\">"));
        WriteLine(pSocket,wxT("<INPUT TYPE=SUBMIT VALUE=\"Rate it\">"));

        WriteLine(pSocket,wxT("&nbsp;"));
        WriteLine(pSocket,wxT("<select name=\"rating\" size=\"1\">\r\n"));
        for(int i = MUSIK_MIN_RATING; i <= MUSIK_MAX_RATING; i ++)
        {

            WriteLine(pSocket,wxString(wxT("<option value=\"")) << i << wxT("\" ")); 
            if(pSong->Rating == i)
                WriteLine(pSocket,wxT(" selected "));
            WriteLine(pSocket,wxString(wxT("> ")) << i << wxT("</option>\r\n")); 
        }
        WriteLine(pSocket,wxT("</select>"));
        WriteLine(pSocket,wxT("</FORM>"));
        WriteLine(pSocket,wxT("</td>\r\n"));
        WriteLine(pSocket,wxT("</tr></table>\r\n"));

		WriteLine(pSocket, wxT("</body>\r\n") );
		WriteLine(pSocket, wxT("</html>\r\n") );	
	return true;
	}	
	return false;
}

// reads a line of input from the socket
int CMusikWebServer::ReadLine(wxSocketBase *pSocket,wxString& outstr)
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
void CMusikWebServer::WriteLine(wxSocketBase*pSocket, const wxString &str )
{
    const wxCharBuffer buf = ConvToUTF8( str );
    pSocket->Write( buf, strlen(buf) );
}

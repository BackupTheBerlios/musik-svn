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
void CMusikWebServer::ProcessRequest(const wxString &reqstr)
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
            WriteLine( wxT("HTTP/1.1 307 Temporary Redirect\r\n") );
            wxString server_version;
            server_version.sprintf( wxT( "Server: %s\r\n" ), MUSIKSERV_VERSION );
            WriteLine( server_version );
            WriteLine( wxT("Location: /\r\n"));
            WriteLine( wxT("\r\n") );

            return;
        }
        WriteLine( wxT("HTTP/1.1 200 OK\r\n") );
        wxString server_version;
        server_version.sprintf( wxT( "Server: %s\r\n" ), MUSIKSERV_VERSION );
        WriteLine( server_version );
        WriteLine( wxT("Content-Type: text/html; charset=UTF8\r\n") );
        WriteLine( wxT("Pragma: no-cache\r\n") );
        WriteLine( wxT("\r\n") );

        bool bIsPlaying = m_pPlayer->IsPlaying();
        bool bIsPaused = m_pPlayer->IsPaused();
        std::auto_ptr<CMusikSong> pSong = m_pPlayer->GetCurrentSongid().Song();

        WriteLine( wxT( "<HTML><HEAD>\r\n") );
        WriteLine( wxString(wxT( "<TITLE>")) << MUSIKAPPNAME_VERSION << wxT("</TITLE>\r\n" ) );
        if(bIsPlaying)
        {
            WriteLine(wxString(wxT("<meta http-equiv=\"Refresh\" content=\"")) 
                << wxGetApp().Prefs.nWebServerRefresh
                << wxT("\">\r\n"));
        }
        WriteLine(wxT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF8\">"));
		WriteLine( wxT("</HEAD><body>\r\n") );

        WriteLine(wxT("<table border=\"0\" cellspacing=\"10\" cellpadding=\"2\" >\r\n"));
        WriteLine(wxT("<tr>"));
        WriteLine(wxT("<td>"));

        WriteLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));

        WriteLine(wxT("<tr><td>\r\n"));
        WriteLine( wxString(_("Title"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Title):wxString(_( "&lt;unknown&gt;"))));
        WriteLine(wxT("</td></tr>\r\n"));

        WriteLine(wxT("<tr><td>\r\n"));
        WriteLine( wxString(_("Artist"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Artist): wxString(_( "&lt;unknown&gt;"))));
        WriteLine(wxT("</td></tr>\r\n"));
        WriteLine(wxT("<tr><td>\r\n"));
        WriteLine( wxString(_("Album"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Album): wxString(_( "&lt;unknown&gt;"))) );
        WriteLine(wxT("</td></tr>\r\n"));
        WriteLine(wxT("<tr><td>\r\n"));
        WriteLine( wxString(_("Rating"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? pSong->Rating : 0) );
        WriteLine(wxT("</td></tr>\r\n"));
        WriteLine(wxT("<tr><td>\r\n"));
        WriteLine( wxString(_("Time"))<< wxT(":</td><td>") << m_pPlayer->GetTimeStr() );
        WriteLine(wxT("</td></tr>\r\n"));
        WriteLine(wxT("</table>\r\n"));
        WriteLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));
        WriteLine(wxT("<tr>\r\n"));
        if(!bIsPlaying)
        {
            WriteLine(wxT("<td>\r\n"));
            WriteLine(wxString(wxT("<a href=\"?play.mkc\" >")) << _("Play") <<wxT("</a>"));
            WriteLine(wxT("</td>\r\n"));
        }
        WriteLine(wxT("<td>\r\n"));
        if(bIsPaused)
            WriteLine(wxString(wxT("<a href=\"?resume.mkc\" >")) << _("Resume") <<wxT("</a>"));
        else
            WriteLine(wxString(wxT("<a href=\"?pause.mkc\" >")) << _("Pause") <<wxT("</a>"));
        WriteLine(wxT("</td>\r\n"));
        if(bIsPlaying)
        {
            WriteLine(wxT("<td>\r\n"));
            WriteLine(wxString(wxT("<a href=\"?stop.mkc\" >")) << _("Stop") <<wxT("</a>"));
            WriteLine(wxT("</td>\r\n"));
        }
        WriteLine(wxT("<td>\r\n"));
        WriteLine(wxString(wxT("<a href=\"?previous.mkc\" >")) << wxT("&lt;&lt;") <<wxT("</a>"));
        WriteLine(wxT("</td>\r\n"));
        WriteLine(wxT("<td>\r\n"));
        WriteLine(wxString(wxT("<a href=\"?next.mkc\" >")) << wxT("&gt;&gt;") <<wxT("</a>"));
        WriteLine(wxT("</td>\r\n"));

        WriteLine(wxT("</tr></table>\r\n"));

        WriteLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >\r\n"));
        WriteLine(wxT("<tr>\r\n"));
        WriteLine(wxT("<td>\r\n"));
        WriteLine(wxString(wxT("<a href=\"?volup.mkc\" >")) << wxT("Vol +") <<wxT("</a>"));
        WriteLine(wxT("</td>\r\n"));
        WriteLine(wxT("<td>\r\n"));
        WriteLine(wxString() << wxGetApp().Prefs.nSndVolume);
        WriteLine(wxT("</td>\r\n"));
        WriteLine(wxT("<td>\r\n"));
        WriteLine(wxString(wxT("<a href=\"?voldown.mkc\" >")) << wxT("Vol -") <<wxT("</a>"));
        WriteLine(wxT("</td>\r\n"));
        WriteLine(wxT("</tr></table>\r\n"));

        WriteLine(wxT("</td><td valign=\"top\">"));
        WriteLine(wxT("<FORM ACTION=\"/rate\">"));
        WriteLine(wxT("<INPUT TYPE=SUBMIT VALUE=\"Rate it\">"));

        WriteLine(wxT("&nbsp;"));
        WriteLine(wxT("<select name=\"rating\" size=\"1\">\r\n"));
        for(int i = MUSIK_MIN_RATING; i <= MUSIK_MAX_RATING; i ++)
        {

            WriteLine(wxString(wxT("<option value=\"")) << i << wxT("\" ")); 
            if(pSong->Rating == i)
                WriteLine(wxT(" selected "));
            WriteLine(wxString(wxT("> ")) << i << wxT("</option>\r\n")); 
        }
        WriteLine(wxT("</select>"));
        WriteLine(wxT("</FORM>"));
        WriteLine(wxT("</td>\r\n"));
        WriteLine(wxT("</tr></table>\r\n"));

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
void CMusikWebServer::WriteLine( const wxString &str )
{
    const wxCharBuffer buf = ConvToUTF8( str );
    pSocket->Write( buf, strlen(buf) );
}

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

void CMusikWebServer::HttpResponse::Send(wxSocketBase*pSocket)
{
    const wxCharBuffer databuf = ConvToUTF8( m_sData );
    int databuflen = strlen(databuf);
    wxString server_version;
    server_version.sprintf( wxT( "Server: %s" ), MUSIKSERV_VERSION );
    AddHeader (server_version );
    AddHeader(wxString( wxT("Content-Length: ")) << databuflen);
    AddHeader(wxT("Connection: close"));
    wxString h;
    h << m_sRC << m_sHeaders << wxT("\r\n");
    const wxCharBuffer hbuf = ConvToUTF8( h );
    pSocket->Write( hbuf, strlen(hbuf) );
    if(databuflen)
        pSocket->Write( databuf, databuflen);
}

void CMusikWebServer::HttpResponse::SetRC(const wxString & rc)
{
    
    m_sRC = wxT("HTTP/1.1 ");
    m_sRC << rc << wxT("\r\n");
}
void CMusikWebServer::HttpResponse::AddDataLine(const wxString & data)
{
    m_sData << data << wxT("\r\n");
}
void CMusikWebServer::HttpResponse::AddHeader(const wxString & Header)
{
   m_sHeaders << Header << wxT("\r\n");
}

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
    pSocket->SetFlags(wxSOCKET_BLOCK|wxSOCKET_WAITALL);// block gui, fix reentrancy problem.
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

            wxString sRequest;
            if(ReadLine(sock, sRequest ))
            {
                wxArrayString arrReqHeaders;
                wxString h;
                while(ReadLine(sock, h )> 2) // > 2 detects lines which do not consist only of \r\n
                {
                    arrReqHeaders.Add(h);
                }
                HttpResponse hr;
                ProcessRequest( sRequest ,hr);
                if(hr.Ok())
                {
                    hr.Send(sock);
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
void CMusikWebServer::ProcessRequest(const wxString &reqstr,HttpResponse &hr)
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
            hr.SetRC(wxT("307 Temporary Redirect"));
            hr.AddHeader(wxT("Location: /"));
            return;
        }
        hr.SetRC(wxT("200 OK"));
        hr.AddHeader(wxT("Content-Type: text/html; charset=UTF8") );

        bool bIsPlaying = m_pPlayer->IsPlaying();
        bool bIsPaused = m_pPlayer->IsPaused();
        std::auto_ptr<CMusikSong> pSong = m_pPlayer->GetCurrentSongid().Song();

        hr.AddDataLine( wxT( "<HTML><HEAD>") );
        hr.AddDataLine( wxString(wxT( "<TITLE>")) << MUSIKAPPNAME_VERSION << wxT("</TITLE>" ) );
        if(bIsPlaying)
        {
            hr.AddDataLine(wxString(wxT("<meta http-equiv=\"Refresh\" content=\"")) 
                << wxGetApp().Prefs.nWebServerRefresh
                << wxT("\">"));
        }
        hr.AddDataLine(wxT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF8\">"));
        hr.AddDataLine( wxT("</HEAD><body>") );

        hr.AddDataLine(wxT("<table border=\"0\" cellspacing=\"10\" cellpadding=\"2\" >"));
        hr.AddDataLine(wxT("<tr>"));
        hr.AddDataLine(wxT("<td>"));

        hr.AddDataLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >"));

        hr.AddDataLine(wxT("<tr><td>"));
        hr.AddDataLine( wxString(_("Title"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Title):wxString(_( "&lt;unknown&gt;"))));
        hr.AddDataLine(wxT("</td></tr>"));

        hr.AddDataLine(wxT("<tr><td>"));
        hr.AddDataLine( wxString(_("Artist"))<< wxT(":</td><td>")  << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Artist): wxString(_( "&lt;unknown&gt;"))));
        hr.AddDataLine(wxT("</td></tr>"));
        hr.AddDataLine(wxT("<tr><td>"));
        hr.AddDataLine( wxString(_("Album"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? ConvFromUTF8(pSong->MetaData.Album): wxString(_( "&lt;unknown&gt;"))) );
        hr.AddDataLine(wxT("</td></tr>"));
        hr.AddDataLine(wxT("<tr><td>"));
        hr.AddDataLine( wxString(_("Rating"))<< wxT(":</td><td>") << (pSong.get()!=NULL ? pSong->Rating : 0) );
        hr.AddDataLine(wxT("</td></tr>"));
        hr.AddDataLine(wxT("<tr><td>"));
        hr.AddDataLine( wxString(_("Time"))<< wxT(":</td><td>") << m_pPlayer->GetTimeStr() );
        hr.AddDataLine(wxT("</td></tr>"));
        hr.AddDataLine(wxT("</table>"));
        hr.AddDataLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >"));
        hr.AddDataLine(wxT("<tr>"));
        if(!bIsPlaying)
        {
            hr.AddDataLine(wxT("<td>"));
            hr.AddDataLine(wxString(wxT("<a href=\"?play.mkc\" >")) << _("Play") <<wxT("</a>"));
            hr.AddDataLine(wxT("</td>"));
        }
        hr.AddDataLine(wxT("<td>"));
        if(bIsPaused)
            hr.AddDataLine(wxString(wxT("<a href=\"?resume.mkc\" >")) << _("Resume") <<wxT("</a>"));
        else
            hr.AddDataLine(wxString(wxT("<a href=\"?pause.mkc\" >")) << _("Pause") <<wxT("</a>"));
        hr.AddDataLine(wxT("</td>"));
        if(bIsPlaying)
        {
            hr.AddDataLine(wxT("<td>"));
            hr.AddDataLine(wxString(wxT("<a href=\"?stop.mkc\" >")) << _("Stop") <<wxT("</a>"));
            hr.AddDataLine(wxT("</td>"));
        }
        hr.AddDataLine(wxT("<td>"));
        hr.AddDataLine(wxString(wxT("<a href=\"?previous.mkc\" >")) << wxT("&lt;&lt;") <<wxT("</a>"));
        hr.AddDataLine(wxT("</td>"));
        hr.AddDataLine(wxT("<td>"));
        hr.AddDataLine(wxString(wxT("<a href=\"?next.mkc\" >")) << wxT("&gt;&gt;") <<wxT("</a>"));
        hr.AddDataLine(wxT("</td>"));

        hr.AddDataLine(wxT("</tr></table>"));

        hr.AddDataLine(wxT("<table border=\"0\" cellspacing=\"0\" cellpadding=\"2\" >"));
        hr.AddDataLine(wxT("<tr>"));
        hr.AddDataLine(wxT("<td>"));
        hr.AddDataLine(wxString(wxT("<a href=\"?volup.mkc\" >")) << wxT("Vol +") <<wxT("</a>"));
        hr.AddDataLine(wxT("</td>"));
        hr.AddDataLine(wxT("<td>"));
        hr.AddDataLine(wxString() << wxGetApp().Prefs.nSndVolume);
        hr.AddDataLine(wxT("</td>"));
        hr.AddDataLine(wxT("<td>"));
        hr.AddDataLine(wxString(wxT("<a href=\"?voldown.mkc\" >")) << wxT("Vol -") <<wxT("</a>"));
        hr.AddDataLine(wxT("</td>"));
        hr.AddDataLine(wxT("</tr></table>"));

        hr.AddDataLine(wxT("</td><td valign=\"top\">"));
        hr.AddDataLine(wxT("<FORM ACTION=\"/rate\">"));
        hr.AddDataLine(wxT("<INPUT TYPE=SUBMIT VALUE=\"Rate it\">"));

        hr.AddDataLine(wxT("&nbsp;"));
        hr.AddDataLine(wxT("<select name=\"rating\" size=\"1\">"));
        for(int i = MUSIK_MIN_RATING; i <= MUSIK_MAX_RATING; i ++)
        {

            hr.AddDataLine(wxString(wxT("<option value=\"")) << i << wxT("\" ")); 
            if(pSong->Rating == i)
                hr.AddDataLine(wxT(" selected "));
            hr.AddDataLine(wxString(wxT("> ")) << i << wxT("</option>")); 
        }
        hr.AddDataLine(wxT("</select>"));
        hr.AddDataLine(wxT("</FORM>"));
        hr.AddDataLine(wxT("</td>"));
        hr.AddDataLine(wxT("</tr></table>"));

        hr.AddDataLine( wxT("</body>") );
        hr.AddDataLine( wxT("</html>") );	
        return;
    }	
    hr.SetRC(wxT("404 Not Found"));
    return ;
}

// reads a line of input from the socket
int CMusikWebServer::ReadLine(wxSocketBase *pSocket,wxString& outstr)
{
    outstr.Empty();
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


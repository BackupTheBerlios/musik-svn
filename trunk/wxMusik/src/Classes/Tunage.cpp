/*
 *  Tunage.cpp
 *
 *  Musik's system for updating websites, files, etc.
 *  with currently-playing info
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#ifdef _WIN32
#include "scrobsub/ScrobSubmitter.h"
#endif
#include "Tunage.h"

#include "MusikGlobals.h"

#include "MusikUtils.h"
#include "wx/url.h"
#include "wx/wfstream.h"
#include "wx/utils.h"



class MusikScrobbler
{

public:	
	MusikScrobbler()
		:m_StopSend(true)
	{
#ifdef _WIN32
		m_submitter.Init("tst",SubmitterCallback,this);
#endif
	}
	~MusikScrobbler()
	{
#ifdef _WIN32
		m_submitter.Term();
#endif

	}

	
	void Start(const CMusikSong& song)
	{
		Stop();
#ifdef _WIN32
		m_submitter.Start(song.MetaData.Artist.c_str()
						 ,song.MetaData.Title.c_str()
						 ,song.MetaData.Album.c_str(),"",song.MetaData.nDuration_ms /1000,"");
#endif
		m_StopSend = false;
	}
	void Resume()
	{
#ifdef _WIN32
		m_submitter.Resume();
#endif
	}
	void Pause()
	{
#ifdef _WIN32
		m_submitter.Pause();
#endif
	}
	void Stop()
	{
		if(!m_StopSend)
		{
#ifdef _WIN32
			m_submitter.Stop();
#endif
			m_StopSend = true;
		}
	}
private:
#ifdef _WIN32
	static	void SubmitterCallback(
		int  reqId,
		bool error,
		std::string msg,
		void* userData)
	{
		MusikScrobbler *pThis = (MusikScrobbler*)userData;

	}

	ScrobSubmitter m_submitter;
#endif
	bool m_StopSend;
};

CTunage::CTunage()
{
	m_pScrobbler = NULL;
}

CTunage::~CTunage()
{
	delete m_pScrobbler;
}


///////////////////////////////////////////////////////////////////////////////

void CTunage::Started(const CMusikSong& song )
{
	if(wxGetApp().Prefs.bTunageUseScrobbler)
	{
		if(!m_pScrobbler)
			m_pScrobbler = new MusikScrobbler();
		m_pScrobbler->Start(song);
	}
	else if (m_pScrobbler)
	{
		delete m_pScrobbler;
		m_pScrobbler = NULL;
	}

	// bail if nothing to do
	if ( !wxGetApp().Prefs.bTunageWriteFile && !wxGetApp().Prefs.bTunagePostURL && !wxGetApp().Prefs.bTunageRunApp )
		return;
	if ( wxGetApp().Prefs.bTunageWriteFile )
		WriteFile(song);

	if ( wxGetApp().Prefs.bTunagePostURL )
		PostURL(song);

	if ( wxGetApp().Prefs.bTunageRunApp )
		RunApp(song);
}
void CTunage::Paused()
{

}


void CTunage::Resumed()
{

}

void CTunage::Stopped()
{
	if ( wxGetApp().Prefs.bTunageRunOnStop)
	{
		CMusikSong stopsong;
		stopsong.MetaData.nFilesize = -1;
		Started( stopsong );
	}
}

///////////////////////////////////////////////////////////////////////////////

void CTunage::PostURL(const CMusikSong& song )
{
	if ( wxGetApp().Prefs.sTunageURL == wxT("") )
		return;

	// create a valid URL
	wxString urltext( wxGetApp().Prefs.sTunageURL );
	ParseTags( urltext , song );

    wxURI uri(urltext);
	wxURL url( uri );
	
	if ( url.GetError() != wxURL_NOERR )
	{
		wxMessageBox( wxT("Could not parse Tunage URL"), wxT("Tunage error"), wxOK | wxICON_WARNING );
		
		return;
	}

	// connect to URL
	wxYield();
	wxInputStream *data = url.GetInputStream();
	if ( !data )
	{
		wxMessageBox( wxT("Could not access URL"), wxT("Tunage error"), wxOK | wxICON_WARNING );
	}

	delete data;



}

///////////////////////////////////////////////////////////////////////////////

void CTunage::WriteFile(const CMusikSong& song )
{
	if ( ( wxGetApp().Prefs.sTunageFilename == wxT("") ) || ( wxGetApp().Prefs.sTunageFileLine == wxT("") ) )
		return;

	if ( !wxGetApp().Prefs.bTunageAppendFile )
	{
		if ( wxFileExists( wxGetApp().Prefs.sTunageFilename ) )
			wxRemoveFile( wxGetApp().Prefs.sTunageFilename );
	}

	wxTextFile Out;	
	Out.Create( wxGetApp().Prefs.sTunageFilename );
	Out.Open();

	if ( !Out.IsOpened() )
		return;

	wxString line( wxGetApp().Prefs.sTunageFileLine );
	ParseTags( line , song );

	Out.AddLine( line );
	Out.Write( Out.GuessType() );
	Out.Close();
}

///////////////////////////////////////////////////////////////////////////////

void CTunage::RunApp(const CMusikSong& song )
{
	if ( wxGetApp().Prefs.sTunageCmdLine == wxT("") )
		return;

	wxString cmd( wxGetApp().Prefs.sTunageCmdLine );
	ParseTags( cmd , song );

	wxExecute( cmd );
}

///////////////////////////////////////////////////////////////////////////////

// replaces special tags such as %TITLE% with info from the song
void CTunage::ParseTags( wxString& str ,const CMusikSong& song )
{
	CNiceFilesize filesize;
	filesize.AddB( song.MetaData.nFilesize );
	wxString sFilesize = filesize.GetFormatted();

	str.Replace( wxT("$ARTIST"), ConvFromUTF8( song.MetaData.Artist ));
	str.Replace( wxT("$ALBUM"), ConvFromUTF8( song.MetaData.Album ));
	str.Replace( wxT("$TITLE"), ConvFromUTF8(song.MetaData.Title ));
	str.Replace( wxT("$YEAR"), ConvFromUTF8(song.MetaData.Year ));

	if ( song.MetaData.nFilesize == -1 )
		str.Replace( wxT("$NAME"), wxGetApp().Prefs.sTunageStoppedText );
	else
		str.Replace( wxT("$NAME"), wxString::Format( wxT("%s - %s"),(const wxChar*)ConvFromUTF8(song.MetaData.Artist), (const wxChar *)ConvFromUTF8(song.MetaData.Title) ) );

	str.Replace( wxT("$FILENAME"), song.MetaData.Filename.GetFullPath() );
	str.Replace( wxT("$FILESIZE"), sFilesize );
	str.Replace( wxT("$BITRATE"), wxString::Format( wxT("%d"), song.MetaData.nBitrate ) );
	str.Replace( wxT("$TRACKLENGTH"), SecToStr( song.MetaData.nDuration_ms /1000 ) );

	str.Replace( wxT("$TIMESPLAYED"), wxString::Format( wxT("%d"), song.TimesPlayed ) );
	str.Replace( wxT("$TRACKNUM"), wxString::Format( wxT("%.2d"), song.MetaData.nTracknum ) );
}


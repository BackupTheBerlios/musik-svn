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

#include "Tunage.h"

#include "MusikGlobals.h"

#include "MusikUtils.h"
#include "wx/url.h"
#include "wx/wfstream.h"
#include "wx/utils.h"



CTunage::CTunage()
{
}

CTunage::~CTunage()
{
}


///////////////////////////////////////////////////////////////////////////////

void CTunage::Execute(const CMusikSong& song )
{
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

void CTunage::Stopped()
{
	if ( wxGetApp().Prefs.bTunageRunOnStop)
	{
		CMusikSong stopsong;
		stopsong.MetaData.nFilesize = -1;
		Execute( stopsong );
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


/*
 *  MusikPrefs.cpp
 *
 *  Class to handle saving / loading
 *  of all preferences.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "MusikPrefs.h"

//--- wx config stuff ---//
#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h> 

#include "../MusikGlobals.h"
#include "../MusikUtils.h"
#include "MusikEQCtrl.h"

#include "MUSIKEngine/inc/equalizer.h"


void CMusikPrefs::LoadPrefs()
{
	Read( wxT("AlternativeDataPath"),	&sDataPath,	MUSIK_HOME_DIR );
	 
	if(sDataPath.IsEmpty())
		  sDataPath = MUSIK_HOME_DIR;
	if(sDataPath.Right(1) != wxFileName::GetPathSeparator())
		   sDataPath += wxFileName::GetPathSeparator();
}


void CMusikPrefs::SavePrefs()
{
}

void CMusikPrefs::LoadBands(MUSIKEqualizer * pEQ )
{
    if(!pEQ)
        return;
    wxConfigBase *pConfig = this;
    MUSIKEqualizer::Bands &lband = pEQ->ChannelBands(MUSIKEqualizer::Bands::Left);
    for ( size_t n = 0; n < lband.Count(); n++ )
    {
        double v = 1.0;
        pConfig->Read(wxString::Format(wxT( "EQL%d" ),n),&v,1.0	);
        lband[n] = v;
    }
    MUSIKEqualizer::Bands &rband = pEQ->ChannelBands(MUSIKEqualizer::Bands::Right);
    for ( size_t n = 0; n < rband.Count(); n++ )
    {
        double v = 1.0;
        pConfig->Read(wxString::Format(wxT( "EQR%d" ),n),&v,1.0	);
        rband[n] = v;
    }
}

wxString CMusikPrefs::GetProxyServer()
{
    if(wxGetApp().Prefs.bUseProxyServer)
    {
        wxString sProxyString(  wxGetApp().Prefs.sProxyServer );
        if(	!wxGetApp().Prefs.sProxyServerPort.IsEmpty() )
        {
            sProxyString += wxT(":") + 	wxGetApp().Prefs.sProxyServerPort;
        }
        if(	!wxGetApp().Prefs.sProxyServerUser.IsEmpty() )	 
        {
            sProxyString =  wxGetApp().Prefs.sProxyServerUser + wxT(":") + wxGetApp().Prefs.sProxyServerPassword + wxT("@")  +  sProxyString;
        }
        return sProxyString;
    }
    else
        return wxEmptyString;
}
void CMusikPaths::Load()
{
	//--- create the file if it doesn't exist ---//
	if ( !wxFileExists( MUSIK_PATHS_FILENAME ) )
	{
		wxTextFile Out;	
		Out.Create( MUSIK_PATHS_FILENAME );
	}

	//--- once the file exists, load it ---//
	wxTextFile In( MUSIK_PATHS_FILENAME );
	In.Open();
	if ( !In.IsOpened() )
		return;

	m_Paths.Clear();
	for ( size_t i = 0; i < In.GetLineCount(); i++ )
	{
		wxString sCheck = In.GetLine( i );
		sCheck.Trim( false ); sCheck.Trim( true );
		m_Paths.Add( In.GetLine( i ) );
	}
	In.Close();
    
	return;
}

void CMusikPaths::Save()
{
	wxRemoveFile( MUSIK_PATHS_FILENAME );

	wxTextFile Out;	
	Out.Create( MUSIK_PATHS_FILENAME );
	Out.Open();
	
	if ( !Out.IsOpened() )
		return;
	
	for ( size_t i = 0; i < m_Paths.GetCount(); i++ )
		Out.AddLine( m_Paths.Item( i ) );

	Out.Write( Out.GuessType() );
	Out.Close();	

	return;
}

/*
 *  MusikPrefsDialog.h
 *
 *  Preferences frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_PREFS_FRAME_H
#define MUSIK_PREFS_FRAME_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif 
#include <wx/treectrl.h>
#include <wx/spinctrl.h>
#include <wx/checklst.h>

#include "../MusikDefines.h"
#include "../Classes/ActivityBox.h"
enum EMUSIK_PREFERENCES_OBJECT_ID
{
	MUSIK_PREFERENCES_TREE = 0,
	MUSIK_PREFERENCES_OUTPUT_DRV,
	MUSIK_PREFERENCES_SND_DEVICE,
	MUSIK_PREFERENCES_PLAY_RATE,
	MUSIK_PREFERENCES_PLAYLIST_STRIPE_COLOUR,
	MUSIK_PREFERENCES_ACTIVITY_STRIPE_COLOUR,
	MUSIK_PREFERENCES_SOURCES_STRIPE_COLOUR,
	MUSIK_PREFERENCES_PLAYLIST_BORDER_COLOUR,
	MUSIK_PREFERENCES_FILE_ASSOC_CHKLB
};

class MusikPrefsDialog : public wxDialog
{
public:
	MusikPrefsDialog( wxWindow* pParent, const wxString &sTitle);

	//--------------//
	//--- events ---//
	//--------------//
	void OnTranslateKeys	( wxKeyEvent	 &event );
	void OnTreeChange		( wxTreeEvent& WXUNUSED(event) );
	void OnOutputChanged	( wxCommandEvent& WXUNUSED(event) );
	void OnClickOK			( wxCommandEvent& WXUNUSED(event) );
	void OnClickApply		( wxCommandEvent& WXUNUSED(event) );
	void OnClickCancel		( wxCommandEvent& WXUNUSED(event) );
	void OnClose			( wxCloseEvent& WXUNUSED(event) );
	void OnClickColour		( wxCommandEvent& event );
protected:
	//-------------------------//
	//--- utility functions ---//
	//-------------------------//
	void LoadPrefs			();
	bool SavePrefs			();
	void FindDevices		();
	void HidePanels			();
	void UpdatePrefsPanel	();
	void Close				( bool bCancel );
	void DoFileAssociations();
	//--------------------//
	//--- tree control ---//
	//--------------------//
	wxTreeCtrl	*tcPreferencesTree;


	//--------------------------//
	//--- options -> general ---//
	//--------------------------//
	
	wxCheckBox	*chkSortArtistWithoutPrefix;
	wxCheckBox	*chkPlaylistStripes;
	wxButton	*btnPlaylistStripeColour;
	wxCheckBox	*chkActivityBoxStripes;
	wxButton	*btnActivityStripeColour;
	wxCheckBox	*chkSourcesBoxStripes;
	wxButton	*btnSourcesStripeColour;

	wxCheckBox	*chkPlaylistBorder;
	wxButton	*btnPlaylistBorderColour;
	//-----------------------------//
	//--- options -> selections ---//
	//-----------------------------//
	wxComboBox	*cmbActivityBoxes[ActivityBoxesMaxCount];
	wxComboBox	*cmbSelStyle;

	//-----------------------------//
	//--- options -> playlist	---//
	//-----------------------------//

	wxCheckBox* chkPLColumnEnable[NPLAYLISTCOLUMNS];

	wxComboBox*	cmbPLColumnStatic[NPLAYLISTCOLUMNS];
	//-------------------------------------//
	//--- options -> file association	---//
	//-------------------------------------//

	wxCheckListBox *chklbFileAssocs;

	//-------------------------//
	//--- options -> tunage ---//
	//-------------------------//

	wxTextCtrl*	tcTunageFilename;
	wxTextCtrl* tcTunageFileLine;
	wxTextCtrl* tcTunageURL;
	wxTextCtrl* tcTunageCmdLine;
	wxTextCtrl* tcTunageStoppedText;

	wxStaticBox *sbTunageFile;
	wxStaticBox *sbTunageURL;
	wxStaticBox *sbTunageApp;
	wxStaticBox *sbTunageMisc;

	//-------------------------//
	//--- options -> Auto DJ ---//
	//-------------------------//

	wxTextCtrl*	tc2AutoDjFilter;

	wxStaticBox *sbShuffle;
	wxStaticBox *sbAutoDj;

	//-------------------------//
	//--- sound -> playback ---//
	//-------------------------//
	wxTextCtrl	*tcDuration;
	wxTextCtrl	*tcSeekDuration;
	wxTextCtrl	*tcPauseResumeDuration;
	wxTextCtrl	*tcStopDuration;
	wxTextCtrl	*tcExitDuration;

	//-----------------------//
	//--- sound -> driver ---//
	//-----------------------//
	wxComboBox	*cmbOutputDrv;
	wxComboBox	*cmbSndDevice;
	wxComboBox	*cmbPlayRate;
	wxTextCtrl	*tcBufferLength;
	wxSpinCtrl * sc2SndMaxChan;
	//---------------------------------//
	//--- streaming -> proxy server ---//
	//---------------------------------//
	wxSpinCtrl* sc2StreamingBufferSize;
	wxSpinCtrl* sc2StreamingPreBufferPercent;
	wxSpinCtrl* sc2StreamingReBufferPercent;
	wxCheckBox* chkUseProxyServer;
	wxTextCtrl*	tcProxyServer;		
	wxTextCtrl* tcProxyServerPort;	
	wxTextCtrl* tcProxyServerUser;	
	wxTextCtrl* tcProxyServerPassword;	

	//--------------//
	//--- sizers ---//
	//--------------//
	wxBoxSizer		*vsSound_Driver;
	wxBoxSizer		*vsSound_Crossfader;
	wxFlexGridSizer	*vsOptions_Selections;
	wxBoxSizer		*vsOptions_Playlist;
	wxBoxSizer		*vsOptions_Interface;
	wxBoxSizer		*vsOptions_FileAssoc;
	wxBoxSizer		*vsOptions_Tunage;
	wxBoxSizer		*vsOptions_AutoDj;
	wxBoxSizer		*vsTagging_General;
	wxBoxSizer		*vsTagging_Auto;
	wxBoxSizer		*hsSplitter;
	wxBoxSizer		*vsTopSizer;
	wxFlexGridSizer	*vsStreaming_Buffer;
	wxBoxSizer		*vsStreaming_ProxyServer;

	DECLARE_EVENT_TABLE()
private:

	//--------------------------//
	//--- selection tracking ---//
	//--------------------------//
	wxTreeItemId nRootID;
	wxTreeItemId nSoundRootID;
	wxTreeItemId nOptionsRootID;
	wxTreeItemId nTagRootID;
	wxTreeItemId nTunageID;
	wxTreeItemId nAutoDjID;
	wxTreeItemId nPlaybackID;
	wxTreeItemId nDriverID;
	wxTreeItemId nSelectionsID;
	wxTreeItemId nPlaylistID;
	wxTreeItemId nFileAssocsID;
	wxTreeItemId nInterfaceID;	
	wxTreeItemId nGeneralTagID;
	wxTreeItemId nAutoTagID;

	wxTreeItemId nStreamingBufferID;
	wxTreeItemId nStreamingProxyServerID;
};

#endif

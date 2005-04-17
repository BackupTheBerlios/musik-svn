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

class wxTreeCtrl;
class wxTreeEvent;
class wxTreeItemId;

class PrefPanel;

class MusikPrefsDialog : public wxDialog
{
public:
	MusikPrefsDialog( wxWindow* pParent, const wxString &sTitle);

	//--------------//
	//--- events ---//
	//--------------//
	void OnTranslateKeys	( wxKeyEvent	 &event );
	void OnTreeChange		( wxTreeEvent& WXUNUSED(event) );
	void OnClickOK			( wxCommandEvent& WXUNUSED(event) );
	void OnClickApply		( wxCommandEvent& WXUNUSED(event) );
	void OnClickCancel		( wxCommandEvent& WXUNUSED(event) );
	void OnClose			( wxCloseEvent& WXUNUSED(event) );
protected:
    void SelectFirstPanel();
    void AddPanel( const wxTreeItemId &tidParent,PrefPanel * panel);

    //-------------------------//
	//--- utility functions ---//
	//-------------------------//
	void LoadPrefs			();
	bool SavePrefs			();
	void Close				( bool bCancel );
	
	//--------------------//
	//--- widgets      ---//
	//--------------------//
	wxTreeCtrl	*tcPreferencesTree;

    wxStaticText *m_stPanelHeader;
	//--------------//
	//--- sizers ---//
	//--------------//
	wxSizer		*m_sizerPanels;

	DECLARE_EVENT_TABLE()
private:
    bool SavePrefs(const wxTreeItemId & root);

    PrefPanel *m_pCurrentPanel;
};

#endif

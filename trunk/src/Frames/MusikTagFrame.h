/*
 *  MusikTagFrame.h
 *
 *  Tag editing frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_TAG_FRAME_H
#define MUSIK_TAG_FRAME_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "wx/thread.h"

//--- CMusikSongArray defined here ---//
#include "../Classes/MusikLibrary.h"
#include "../ThreadController.h"

class MusikTagApplyThread;

enum EMUSIK_TAG_OBJECT_ID
{
	MUSIK_TAG_TITLE = 0,
	MUSIK_TAG_TRACKNUM,
	MUSIK_TAG_ARTIST,
	MUSIK_TAG_ALBUM,
	MUSIK_TAG_GENRE,
	MUSIK_TAG_YEAR,
	MUSIK_TAG_NOTES,
	MUSIK_TAG_CHK_TITLE,
	MUSIK_TAG_CHK_TRACKNUM,
	MUSIK_TAG_CHK_ARTIST,
	MUSIK_TAG_CHK_ALBUM,
	MUSIK_TAG_CHK_GENRE,
	MUSIK_TAG_CHK_YEAR,
	MUSIK_TAG_CHK_NOTES,
	MUSIK_TAG_CHK_WRITETAG,
	MUSIK_TAG_CHK_WRITETAG_CLEAR,
	MUSIK_TAG_CHK_RENAME,
	MUSIK_TAG_SINGLE,
	MUSIK_TAG_MULTIPLE,
	MUSIK_TAG_CHECK,
	MUSIK_TAG_CANCEL,
	MUSIK_TAG_APPLY,
	MUSIK_TAG_OK,
	MUSIK_TAG_NEXT,
	MUSIK_TAG_PREV
};

const EMUSIK_TAG_OBJECT_ID	MUSIK_TAG_CHK_TAGFIRST =  MUSIK_TAG_CHK_TITLE;
const EMUSIK_TAG_OBJECT_ID	MUSIK_TAG_CHK_TAGLAST =  MUSIK_TAG_CHK_NOTES;

class  CPlaylistCtrl;

class MusikTagFrame : public wxFrame
{
public:
	MusikTagFrame( wxFrame* pParent, CPlaylistCtrl * playlistctrl, int nCurFrame);

	//--- objects ---//
	wxTextCtrl  *tcFilename;
	wxTextCtrl	*tcTitle;
	wxCheckBox	*chkTitle;
	wxTextCtrl	*tcTrackNum;
	wxCheckBox	*chkTrackNum;
	wxTextCtrl	*tcArtist;
	wxCheckBox	*chkArtist;
	wxTextCtrl	*tcAlbum;
	wxCheckBox	*chkAlbum;
	wxTextCtrl	*tcYear;
	wxCheckBox	*chkYear;
	wxTextCtrl	*tcNotes;
	wxCheckBox	*chkNotes;

	wxCheckBox  *chkWriteTag;
	wxCheckBox  *chkClear;
	wxCheckBox	*chkRename;

	wxComboBox	*cmbGenre;
	wxCheckBox	*chkGenre;

	wxGauge		*gProgress;

	wxButton	*btnCancel;
	wxButton	*btnApply;
	wxButton	*btnOK;
	wxButton	*btnNext;
	wxButton	*btnPrev;

	wxBoxSizer	*hsRowProgress;
	wxBoxSizer	*vsRows;
	wxBoxSizer	*hsNav;
	//--- regular event handlers ---//
	void OnClickOK				( wxCommandEvent &WXUNUSED(event) );
	void OnClickCancel			( wxCommandEvent &WXUNUSED(event) );
	void OnClickApply			( wxCommandEvent &WXUNUSED(event) ) { Apply(); 							}
	void OnClickCheckTags		( wxCommandEvent &event ) { nFrame = event.GetId() - MUSIK_TAG_CHK_TAGFIRST ; SetEnabled();			}

	void OnClickNext			( wxCommandEvent &WXUNUSED(event) ) { Next();								}
	void OnClickPrev			( wxCommandEvent &WXUNUSED(event) ) { Prev();								}
	void OnClose				( wxCloseEvent &WXUNUSED(event) ); 
	void OnTranslateKeys		( wxKeyEvent& event		);

	//--- thread event handlers ---//
	void OnTagThreadStart	( wxCommandEvent &WXUNUSED(event) );
	void OnTagThreadEnd		( wxCommandEvent &WXUNUSED(event) );	
	void OnTagThreadProg	( wxCommandEvent &WXUNUSED(event) );	

	//--- regular functions ---//
	void PopulateTagDlg	();
	void SetFocus		();
	void SetChecks		( const int i );
	void SetEnabled		();
	void Apply			( bool close = false );
	void SaveCurSong	();
	void SaveSong		(int n);
	void Next			();
	void Prev			();
	void SetCaption		();

	void CheckChangesBatch	();

	//--- virtual overrides ---//
	virtual bool Show( bool show = true );

	void Close();
	//--- thread related functions ---//
	CMusikSongArray* GetSongs	()						{ return &m_Songs;			}

	void EnableProgress			( bool enable = true );


	DECLARE_EVENT_TABLE()
private:

	CThreadController m_ActiveThreadController;

	CMusikSongArray & m_Songs;
	bool			m_WriteTag;
	int				nIndex;
	int				nFrame;

	int m_EditType;
	
	wxArrayInt m_arrSongsSelected;
	//--- thread related stuff ---//
	int m_ProgressType;


	bool m_Close;
	bool m_bDirty;
};

#endif

/*
 *  MusikLibraryDialog.h
 *
 *  Library setup frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_LIBRARY_FRAME_H
#define MUSIK_LIBRARY_FRAME_H

//--- wx ---//
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "wx/listctrl.h"
#include "wx/thread.h"

#include "../ThreadController.h"

//--- forward declarations for threads ---//
class MusikUpdateLibThread;
class MusikPurgeLibThread;
class MusikScanNewThread;

enum EMUSIK_Library_OBJECT_ID
{
	MUSIK_PATHS_LIST = 0,
};

class MusikLibraryDialog: public wxDialog
{
public:

	MusikLibraryDialog( wxWindow* pParent ,const wxArrayString &arrFilenamesToScan = wxArrayString(),unsigned long flags = 0);
	MusikLibraryDialog( wxWindow* pParent, const wxPoint &pos, const wxSize &size );

	//--- functions ---//
	void CreateControls ();
	void PathsLoad		();
	void PathsSave		();
	void ClearLibrary	();
	void UpdateLibrary	( bool bConfirm, bool bCompleteRebuild = false );
	void ScanNew		();
	void PurgeLibrary	();
	void EnableProgress	( bool );
	bool ValidatePath	( wxString sPath );
	
	//--- regular events ---//
	void OnClickAdd				( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListAdd();												}
	void OnClickRemoveSel		( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListRemoveSel();											}
	void OnClickRemoveAll		( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListRemoveAll();											}
	void OnClose				( wxCloseEvent  &WXUNUSED(event)	)		{ Close( true );												}
	void OnSize					( wxSizeEvent	  &WXUNUSED(event)	)		{ PathsResize();												}
	void OnKeyPress				( wxListEvent	  &event			)		{ PathsListProcessKeys( event );								}
	void OnClickOK				( wxCommandEvent  &WXUNUSED(event)	)		{ Close( false );												}
	void OnClickCancel			( wxCommandEvent  &WXUNUSED(event)	)		
	{
			Close( true );	
	}
	void OnClickClearLibrary	( wxCommandEvent  &WXUNUSED(event)	)		{ ClearLibrary();												}
	void OnClickScan			( wxCommandEvent  &WXUNUSED(event)	)		{ ScanNew();													}
	void OnUpdateAll			( wxCommandEvent  &WXUNUSED(event)	);
	void OnRebuildAll			( wxCommandEvent  &WXUNUSED(event)	);
	void OnPurgeLibrary			( wxCommandEvent  &WXUNUSED(event)	)		{ PurgeLibrary();												}
	void TranslateKeys			( wxKeyEvent	  &WXUNUSED(event)	);

	//--- events we get from threads ---//
	void OnThreadStart			( wxCommandEvent& event );
	void OnThreadEnd			( wxCommandEvent& event );
	void OnThreadProg			( wxCommandEvent& event );
	void OnThreadScanProg		( wxCommandEvent& event );
	
	//--- overrides ---//
	virtual bool Show( bool show = true );

	void SetProgress			( int n )			{ m_Progress = n;			}
	void SetProgressType		( int n )			{ m_ProgressType = n;		}

	size_t  GetProgress			()	{ return m_Progress;		}
	size_t  GetProgressType		()	{ return m_ProgressType;	}
	
	size_t	GetTotal			()	{ return m_Total;			}

	void SetTotal				( size_t n )	{ m_Total = n;		}
	void SetNew					( long n )	{ m_New = n;		}

	wxListCtrl		*lcPaths;
	wxGauge			*gProgress;
	wxButton		*btnOK;
	wxButton		*btnCancel;
	wxBoxSizer		*hsSysButtons;
	wxGridSizer		*hsLibraryButtons;
	wxBoxSizer		*vsTopSizer;

    void PathsListRemoveSel		();
	void PathsListRemoveAll		();
	void PathsListAdd			();
	void PathsListProcessKeys	( wxListEvent &event );
	void Close					( bool bCancel );
	void PathsResize			();
	void PathsPopupMenu			( wxContextMenuEvent& event );
	void Activate				();
	void PathsGetSel	( wxArrayString &aReturn );

	wxMenu *paths_setup_menu;
	wxMenu *paths_update_menu;
	wxMenu *paths_context_menu;

	DECLARE_EVENT_TABLE()
private:
	wxStopWatch m_StopWatch;
	//--- variables the threads use to talk to the main ui ---//
	int m_Progress;
	int m_ProgressType;

	CThreadController m_ActiveThreadController;

	wxArrayString m_arrScannedFiles;
	size_t m_Total;
	long m_New;
	size_t m_ScanCount;
	

	bool m_FirstStart;
	bool m_AutoStart;
	bool m_Close;
	unsigned long m_flagsUpdate;
	wxString m_Title; //--- so it doesn't have to be recreated millions of times ---//

	wxArrayString aDelDirs;
	bool bRebuild;
};

#endif

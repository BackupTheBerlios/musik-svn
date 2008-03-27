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
#include "myprec.h"
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

class MusikLibraryDialog: public MusikDialog
{
public:
    MusikLibraryDialog()
    {
        Init();
    }
	MusikLibraryDialog(wxString &sPersistData, wxWindow* pParent ,const wxArrayString &arrFilenamesToScan = wxArrayString(),unsigned long flags = 0)
		:MusikDialog(sPersistData)
	{
        Init();
        Create(  pParent ,arrFilenamesToScan,flags);
    }
	MusikLibraryDialog(wxString &sPersistData, wxWindow* pParent, const wxPoint &pos, const wxSize &size )
		:MusikDialog(sPersistData)
    {
        Init();
        Create(  pParent ,pos,size);
    }
    void Init();
    bool Create( wxWindow* pParent ,const wxArrayString &arrFilenamesToScan = wxArrayString(),unsigned long flags = 0);
    bool Create( wxWindow* pParent, const wxPoint &pos, const wxSize &size );
    //--- overrides ---//
    virtual bool Show( bool show = true );
protected:
	//--- functions ---//
	void CreateControls ();
	void PathsLoad		();
	void PathsSave		();
	void ClearLibrary	();
	void UpdateLibrary	( bool bConfirm ,unsigned long flags = 0);
	void ScanNew		();
	void EnableProgress	( bool );
	bool ValidatePath	( wxString sPath );
	
	//--- regular events ---//
	void OnClickAdd				( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListAdd();												}
	void OnClickRemoveSel		( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListRemoveSel();											}
	void OnUpdateUIRemoveSel	( wxUpdateUIEvent & event);
	void OnClickRemoveAll		( wxCommandEvent  &WXUNUSED(event)	)		{ PathsListRemoveAll();											}
	void OnClose				( wxCloseEvent  &WXUNUSED(event)	)		{ Close( true );												}
	void OnSize					( wxSizeEvent	  &WXUNUSED(event)	)		{ PathsResize();												}
	void OnListKeyDown			( wxListEvent	  &event			);
	void OnClickOK				( wxCommandEvent  &WXUNUSED(event)	)		{ Close( false );												}
	void OnClickCancel			( wxCommandEvent  &WXUNUSED(event)	)		
	{
			Close( true );	
	}
	void OnClickClearLibrary	( wxCommandEvent  &WXUNUSED(event)	)		{ ClearLibrary();												}
	void OnClickScan			( wxCommandEvent  &WXUNUSED(event)	)		{ ScanNew();													}
	void OnUpdateAll			( wxCommandEvent  &WXUNUSED(event)	);
	void OnRebuildAll			( wxCommandEvent  &WXUNUSED(event)	);
	void TranslateKeys			( wxKeyEvent	  &WXUNUSED(event)	);

	//--- events we get from threads ---//
	void OnThreadStart			( wxCommandEvent& event );
	void OnThreadEnd			( wxCommandEvent& event );
	void OnThreadProg			( wxCommandEvent& event );
	void OnThreadScanProg		( wxCommandEvent& event );
	

	void SetProgress			( int n )			{ m_Progress = n;			}
	void SetProgressType		( int n )			{ m_ProgressType = n;		}

	size_t  GetProgress			()	{ return m_Progress;		}
	size_t  GetProgressType		()	{ return m_ProgressType;	}
	
	size_t	GetTotal			()	{ return m_Total;			}

	void SetTotal				( size_t n )	{ m_Total = n;		}
	void SetNew					( long n )	{ m_New = n;		}

	wxListCtrl		*lcPaths;
	wxGauge			*gProgress;
	wxStdDialogButtonSizer	*hsSysButtons;
    wxSizer         *sizerPaths;
	wxBoxSizer		*hsLibraryButtons;
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

    DECLARE_DYNAMIC_CLASS(MusikLibraryDialog)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(MusikLibraryDialog)
private:
	wxStopWatch m_StopWatch;
	//--- variables the threads use to talk to the main ui ---//
	int m_Progress;
	int m_ProgressType;

	wxArrayString m_arrScannedFiles;
	size_t m_Total;
	long m_New;
	size_t m_ScanCount;
	

	bool m_FirstStart;
	bool m_AutoStart;
	bool m_Close;
	unsigned long m_flagsUpdate;
	wxString m_Title; //--- so it doesn't have to be recreated millions of times ---//

	bool m_bRebuild;
    // 
    CThreadController m_ActiveThreadController; // this should be the last member, so it is destructed last

};

#endif

/*
 *  MusikFrame.h
 *
 *  Musik's primary frame
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_FRAME_H
#define MUSIK_FRAME_H

//--- objects ---//
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/gauge.h>
#include <wx/socket.h>
#ifdef wxHAS_TASK_BAR_ICON
#include "wx/taskbar.h"
#ifdef __WXMSW__
#include <ShellAPI.h>
//
// Here i define wxTaskBarIconWindow as it is only define privately in taskbar,cpp of wxwidgets
// This is a hack, but which choice do i have else?
class wxTaskBarIconWindow : public wxFrame
{
public:
	wxTaskBarIconWindow(wxTaskBarIcon *icon)
		: wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0),
		m_icon(icon)
	{
	}


private:
	wxTaskBarIcon *m_icon;
};

#endif


class wxSashEvent;
class wxSashLayoutWindow;

class CNowPlayingCtrl;
class MusikLibraryFrame;
class CMusikSong;
class MusikWriteDirtyThread;

//quite hackish attempt to be able to compile with release 2.5.2 and cvs HEAD before 2.5.3 release
#if wxVERSION_NUMBER < 2503
#ifndef wxSUBRELEASE_NUMBER
#define wxTaskBarIconEvent wxEvent
#endif
#endif
class MusikTaskBarIcon: public wxTaskBarIcon
{
public:
	MusikTaskBarIcon(wxFrame * frame);
	virtual bool SetIcon(const wxIcon& icon,
		const wxString& tooltip = wxEmptyString);
#ifdef __WXMSW__
	virtual bool ShowBalloonInfo(const wxString &sTitle,const wxString & sText);
#endif
	void RestoreFrame();
    void OnRButtonUp(wxTaskBarIconEvent&);
    void OnLButtonDown(wxTaskBarIconEvent&);
    void OnMenuRestore(wxCommandEvent&);
    void OnMenuHide(wxCommandEvent&);
    void OnMenuPlayPause(wxCommandEvent&);
    void OnMenuPrev(wxCommandEvent&);
    void OnMenuNext(wxCommandEvent&);
    void OnMenuStop(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    void OnMenuSetNewIcon(wxCommandEvent&);
	void OnUpdateUIRateSel ( wxUpdateUIEvent &event);
	void OnMenuRateSel( wxCommandEvent& event );

DECLARE_EVENT_TABLE()

private:
	wxFrame *m_pFrame;
#ifdef __WXMSW__
	DWORD m_dwShellDllVersion;
#endif
};

#endif

class MusikFrame : public wxFrame
{
public:
	MusikFrame();
	~MusikFrame();

	//--------------//	
	//--- events ---//
	//--------------//
	void OnMove						( wxMoveEvent&	WXUNUSED(event) );
	void OnMaximize					( wxMaximizeEvent&	WXUNUSED(event) );
	void OnIconize					( wxIconizeEvent&	WXUNUSED(event) );
	void OnSize						( wxSizeEvent	&	WXUNUSED(event) );
	void OnClose					( wxCloseEvent&		WXUNUSED(event) );
	void OnMenuClose				( wxCommandEvent&	WXUNUSED(event) ){ Close();};
	void OnMenuAbout				( wxCommandEvent&	event);

	void OnSetupPaths				( wxCommandEvent&	WXUNUSED(event) );
	void OnPreferences				( wxCommandEvent&	event			);
	void OnFX						( wxCommandEvent&	event			);

	void OnStayOnTop				( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateUIStayOnTop		( wxUpdateUIEvent&	event			);

	void OnSourcesState				( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateUISourcesState		( wxUpdateUIEvent&	event			);
	void OnActivitiesState			( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateUIActivitiesState	( wxUpdateUIEvent&	event			);
	void OnPlaylistInfoState		( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateUIPlaylistInfoState( wxUpdateUIEvent&	event			);
	void OnNowPlayingControlOnTopState( wxCommandEvent& WXUNUSED(event) );
	void OnUpdateUINowPlayingControlOnTopState	( wxUpdateUIEvent&	event			);
	void OnAlbumartState			( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateUIAlbumartState	( wxUpdateUIEvent&	event			);

	void OnSelectSources			( wxCommandEvent&	event );

	void OnCustomQuery				( wxCommandEvent&	WXUNUSED(event) );

	void OnViewDirtyTags			( wxCommandEvent&	WXUNUSED(event) );
	void OnWriteTags				( wxCommandEvent&	WXUNUSED(event) );
	void OnWriteTagsClearDirty		( wxCommandEvent&	WXUNUSED(event) );

	void OnTranslateKeys			( wxKeyEvent&		event			);
	void OnServerEvent				( wxSocketEvent&	event			);
	
	void OnStartProgress			( wxCommandEvent&	WXUNUSED(event) );
	void OnUpdateProgress			( wxCommandEvent&	WXUNUSED(event) );
	void OnEndProgress				( wxCommandEvent&	WXUNUSED(event) );
	
	void OnSashDraggedSourcesBox	(wxSashEvent & ev);
	void OnSashDraggedActivityCtrl	(wxSashEvent & ev);
	void OnEraseBackground ( wxEraseEvent& event );
	//-------------------------//
	//--- virtual overrides ---//
	//-------------------------//
	virtual void SetTitle(const wxString& title = wxEmptyString);
	virtual void SetSongInfoText(const CMusikSong& song);
	virtual void SetSongInfoText(const wxString & sSongInfoText);

	//------------------------------//
	//---     activity boxes     ---//
	//---  playlist info control ---//
	//---        playlist        ---//
	//---    playlist control    ---//
	//---    now playing info    ---//
	//---      and sources       ---//
	//--- defined in the globals ---//
	//------------------------------//

	//--- sources bitmaps ---//
	wxBitmap		bmpLibrary;
	wxBitmap		bmpPlaylist;
	wxBitmap		bmpDynamic;
	wxBitmap		bmpNowplaying;
	wxBitmap		bmpNetstream;

	

	//--- sizers ---//
	wxBoxSizer *vsTopBottom;

	CNowPlayingCtrl		*m_pNowPlayingCtrl;

	//----------------------------------------------------------------//
	//--- Routines dealing with events. List box's clicked, menu   ---//
	//--- items selected, playlist buttons pushed, dialog resized, ---//
	//--- activating, etc. Implmentation for these routines will   ---//
	//--- be will be found in MusikDlgEvents.cpp.                  ---//
	//----------------------------------------------------------------//

	//--- library ---//
	void	LibraryCustomQuery		();

	void	AutoUpdate	( const wxArrayString & Filenames = wxArrayString(),unsigned long flags = 0);
	//--- tag related ---//
	void	WriteTags				();

	//--------------------------------------------------------------------//
	//--- User interface routines that deal with creating controls     ---//
	//--- on the fly and the like. Implementation for these will be    ---//
	//--- found in MusikFrame.cpp							           ---//
	//--------------------------------------------------------------------//

	//--- custom layout ---//
	void TogglePlaylistInfo	();
	void ShowPlaylistInfo	();
	void ToggleSources		();
	void ToggleActivities	();
	void ShowSources		();
	void ShowAlbumArt		();
	void ShowActivityArea	( bool bShow = true );
	void EnableProgress		( bool show = true );
	void SetStayOnTop( bool bStayOnTop );

	//--- images and fonts ---//
	void GetFonts();
	void GetListCtrlFont();
	void LoadImageLists();
	void DeleteImageLists();

	//--- sets ---//
	void SetProgress			( int n )				{ m_Progress = n;			}
	void SetProgressType		( int n )				{ m_ProgressType = n;		}
	void SetActiveThread		( wxThread* thread )	{ m_ActiveThread = thread;	}

	//--- gets ---//
	int  GetProgress			()						{ return m_Progress;		}
	int  GetProgressType		()						{ return m_ProgressType;	}
	wxThread* GetActiveThread	()						{ return m_ActiveThread;	}

	DECLARE_EVENT_TABLE()
protected:
	void CreateMainMenu();
#ifdef __WXMSW__
protected:
    	long MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
private:
#ifdef wxHAS_TASK_BAR_ICON
	MusikTaskBarIcon* m_pTaskBarIcon;
#endif
	//--- threads and thread related ---//
	int m_Progress;
	int m_ProgressType;
	wxThread* m_ActiveThread;
	MusikWriteDirtyThread* pWriteDirtyThread;
	
	wxGauge	  *	m_pProgressGauge;
	wxSashLayoutWindow *m_pBottomPanel;
};

#endif

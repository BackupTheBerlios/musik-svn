/*
 *  ActivityAreaCtrl.h
 *
 *  The "Activity Area" control
 *	This control puts all the activity boxes next to each other
 *	and manages creation / deletion of them, as well as events.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_ACTIVITYAREA_CTRL
#define MUSIK_ACTIVITYAREA_CTRL

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 
#include "wx/laywin.h"
#include "ActivityBox.h"

class CActivityAreaCtrl : public wxSashLayoutWindow
{
public:
	CActivityAreaCtrl( wxWindow *pParent );
	~CActivityAreaCtrl();

	//--- events ---//
	void OnActivityBoxFocused		( wxListEvent& event);
	void OnActivityBoxSelected		( wxListEvent& event);
	void OnActivityBoxColResize		( wxListEvent& event ){ event.Veto(); }
	void OnActivityBoxActivated		( wxListEvent& event );

	void OnSashDragged				(wxSashEvent & ev);
	void OnSize						( wxSizeEvent& event );
	//--- member functions ---//
	void UpdateSel( CActivityBox *pSel );
	bool Create();
	void Delete();
	void ResetAllContents( bool bUpdatePlaylist = true );
	CActivityBox* GetActivityBox(EMUSIK_ACTIVITY_TYPE eType);
	bool Show(bool show = true);
	void SetParent( int ACTIVITY_BOX_ID, bool bUpdate = false );

	int GetParentId(){ return m_ParentId; }
	CActivityBox* GetParentBox(){ return m_ParentBox; }

	DECLARE_EVENT_TABLE()

private:
	int m_ParentId;
	CActivityBox *m_ParentBox;
	CActivityBox *m_pLastSelectedBox;
	CActivityBox *m_ActivityBox[ActivityBoxesMaxCount];		
	wxBoxSizer* pTopSizer;
	wxPanel	  * m_pPanel;
	bool m_bFocused; // this flags are used, to make instant selection working
	bool m_Selected;
	bool m_Selecting;
	bool m_UpdatePlaylist;
	bool m_bContentInvalid;
};

#endif

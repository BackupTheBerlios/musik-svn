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

#include "wx/laywin.h"
#include "ActivityBox.h"

class CActivityAreaCtrl : public wxSashLayoutWindow
{
public:
	CActivityAreaCtrl( wxWindow *pParent );
	~CActivityAreaCtrl();

	//--- events ---//
	void OnActivityBoxColResize		( wxListEvent& event ){ event.Veto(); }
	void OnActivityBoxSelChanged	( wxCommandEvent& event );
    void OnActivityBoxActivated     (  wxListEvent& event );
	void OnSashDragged				(wxSashEvent & ev);
	void OnSize						( wxSizeEvent& event );
	//--- member functions ---//
	void UpdateSel( CActivityBox *pSel ,bool bForceShowAll = false);
	bool ReCreate();
	void ResetAllContents( );
    void ReloadAllContents(  );
    CActivityBox* GetActivityBox(size_t nIndex);
    CActivityBox* GetActivityBox(PlaylistColumn::eId ColId);
    size_t GetActivityBoxCount();
	bool Show(bool show = true);
	void SetParent( CActivityBox* pBox, bool bUpdate = false );
	CActivityBox* GetParentBox(){ return m_ParentBox; }

	DECLARE_EVENT_TABLE()

protected:
    bool Create();
    void Delete();

private:
	int m_ParentId;
	CActivityBox *m_ParentBox;
	CActivityBox *m_pLastSelectedBox;
	CActivityBox *m_ActivityBox[ActivityBoxesMaxCount];		
	wxBoxSizer* pTopSizer;
	wxPanel	  * m_pPanel;
	bool m_bContentInvalid;
};

#endif

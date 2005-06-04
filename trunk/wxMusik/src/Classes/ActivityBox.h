/*
 *  ActivityBox.cpp
 *
 *  The infamous "Activity Box" control
 *	These controls are visible at the top of the main dialog.
 *	They contain lists of artists / albums / genres / etc.
 *
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef ACTIVITY_BOX
#define ACTIVITY_BOX

#include "myprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif 

#include "wx/listctrl.h"
#include "wx/thread.h"
#include "wx/dnd.h"

#include "../MusikDefines.h"
#define MUSIK_ACT_TEXT 9998

#include "../ThreadController.h"
//--- for MusikSongIdArray ---//
#include "MusikLibrary.h"

#include "MusikListCtrl.h"


enum eResetContentMode{RCM_None,RCM_DeselectAll,RCM_EnsureVisibilityOfCurrentTopItem,RCM_PreserveSelectedItems};


//--- forward declarations ---//
class CActivityBox;
class MusikActivityRenameThread;



class CActivityListBox : public CMusikListCtrl
{
public:
	CActivityListBox( CActivityBox *parent, wxWindowID id );
	~CActivityListBox();

	//--- gets ---//
	wxString				GetFirstSel	();
	void					GetSelected	( wxArrayString & aReturn );
	const wxArrayString&	GetList		() const { return m_Items; }

	//--- sets ---//
	void SetCaption( const wxString & sCaption );
	void SetList		(const  wxArrayString & aList,enum eResetContentMode rcm);
	void SetSel			( const  wxArrayString & aList );
	void SetSel			( const wxString & sel, bool bEnsureVisible = true,bool bDeselectAllFirst = true);
	void SetRelated		( int n );

	//--- others ---//
	bool IsSelected		( int n );
	void Update			( bool selectnone );

	void OnChar( wxKeyEvent& event );

  DECLARE_EVENT_TABLE()
protected:
	void RescaleColumns	();	
	bool OnRescaleColumns() { RescaleColumns();return true;}
    wxMenu * CreateContextMenu();	

	void ScrollToItem(const wxString & sItem, bool bCenter = true,bool bSelectItem = false);
    void ScrollToItem(long nItem, bool bCenter = true,bool bSelectItem = false);
private:
	//--- virtual functions ---//
    virtual	wxString		OnGetItemText	(long item, long column) const;
    virtual	wxListItemAttr*	OnGetItemAttr	(long item) const;
	virtual	int				OnGetItemImage	(long WXUNUSED(item)) const { return 0; }

	bool		HasShowAllRow	() const; 
	size_t			GetRowCount		() const { return HasShowAllRow()?( size_t )m_Items.GetCount()+ 1: ( size_t )m_Items.GetCount(); }
	inline wxString GetRowText		( long row, bool bPure = true ) const ;

	void RefreshCaption();
	//--- stripe colors ---//
	wxListItemAttr m_LightAttr;
	wxListItemAttr m_DarkAttr;
	wxListItemAttr m_ActiveAttr;
	wxListItemAttr m_AllReset;
	
	int m_Related;
	wxStopWatch m_OnCharStopWatch;
	wxString m_sSearch;
	wxArrayString m_Items;
	CActivityBox *m_pParent;
	wxString m_sCaption;
	bool m_bIgnoreSetItemStateEvents;
};

class CActivityBoxEvt : public wxEvtHandler
{
public:
	CActivityBoxEvt( CActivityBox *parent, CActivityListBox *box ){ pParent = parent; pListBox = box; };
	~CActivityBoxEvt(){};

	void TranslateKeys	( wxKeyEvent& event	);
	DECLARE_EVENT_TABLE()
private:
	CActivityBox	 *pParent;
	CActivityListBox *pListBox;
};

class CActivityEditEvt : public wxEvtHandler
{
public:
	CActivityEditEvt( CActivityBox *parent, wxTextCtrl *textctrl ){ pParent = parent; pTextCtrl = textctrl; };
	~CActivityEditEvt(){};
	void TranslateKeys( wxKeyEvent& event );
	void EditCommit();
	
	DECLARE_EVENT_TABLE()
private:
	CActivityBox	*pParent;
	wxTextCtrl		*pTextCtrl;
};


class CActivityBox : public wxPanel
{
public:
    CActivityBox( wxWindow *parent, wxWindowID id, PlaylistColumn::eId Type );
	~CActivityBox();
	
	void OnActivityBoxSelDrag( wxListEvent& WXUNUSED(event) )
	{	
		DNDBegin();		
	}
	//--------------//
	//--- playing ---//
	//--------------//
	void OnPlayInstantly( wxCommandEvent& event );	
	void OnPlayAsNext	( wxCommandEvent& event );
	void OnPlayEnqueued	( wxCommandEvent& event );
	void OnPlayReplaceWithSel( wxCommandEvent& event );
   
    void OnListItemMiddleClick( wxListEvent & event);

	//other
	void OnRename(wxCommandEvent& WXUNUSED(event)) { EditBegin();}
	//--- thread event handlers ---//
	void StartRenameThread		( int mode, const wxArrayString & sel, wxString newvalue );
	void OnRenameThreadStart	( wxCommandEvent& WXUNUSED(event) );
	void OnRenameThreadEnd		( wxCommandEvent& WXUNUSED(event) );
	void OnRenameThreadProg		( wxCommandEvent& WXUNUSED(event) );

	//--- sets ---//
	void SelectNone			    ()															{ pListBox->SelectNone();								}
	void SetSel					( const wxString & sel, bool bDeselectAllFirst = true )		{ pListBox->SetSel( sel , bDeselectAllFirst );			}
	void SetSel					( const  wxArrayString & aList )							{ pListBox->SetSel( aList );							}	
	void SetDropTarget			(wxDropTarget* target)										{ pListBox->SetDropTarget( target );					}	
	
	//--- gets ---//
	int			GetListId				()													{ return pListBox->GetId();								}
	wxString	GetFirstSel				()													{ return pListBox->GetFirstSel( ); 						}
	void		GetSelected				( wxArrayString & aReturn )							{ pListBox->GetSelected( aReturn );return; 				}
	int			GetSelectedItemCount	()													{ return pListBox->GetSelectedItemCount();				}
	void		GetRelatedList			( CActivityBox *pParentBox, wxArrayString & aReturn );
	wxString	TypeAsTranslatedString		();
	void		GetSelectedSongs		( MusikSongIdArray& array );

	CActivityListBox*	GetListBox	()		{ return pListBox;		}
    PlaylistColumn::eId	Type	()			{ return m_ActivityType;}

	//--- tag editing ---//
	void EditBegin			();
	void EditCommit			();
	void EditCancel			();
	void EditDone			();
	bool EditVisible		(){ return m_EditVisible ; }

	//--- drag 'n drop ---//
	void		DNDBegin			();
	wxString	DNDGetList			();

	//---misc ---//
	bool IsSelected			( int n )							{ return pListBox->IsSelected( n );		}
	void Update				( bool selectnone = true )			{ pListBox->Update( selectnone );		}	
    void ReloadContents     ();
    

    void ResetContents		( enum eResetContentMode rcm = RCM_DeselectAll);
	void SetContents		( const wxArrayString & aList ,  enum eResetContentMode rcm = RCM_DeselectAll); 
	void GetFullList		( wxArrayString & aReturn, bool bSorted = true );
	void SetPlaylist		();
	void EnableProgress		( bool enable = true );
	void SetRelated			( int n );

	//--- custom, thread event handlers ---//
	void SetProgress			( int n )				{ m_Progress = n; }
	void SetProgressType		( int n )				{ m_ProgressType = n;		}

	int GetProgress()			{ return m_Progress;		}
	int GetProgressType()		{ return m_ProgressType;	}

	DECLARE_EVENT_TABLE()
protected:
	wxMenu * CreateContextMenu();
private:

    PlaylistColumn::eId m_ActivityType;
	bool m_EditVisible;

	wxBoxSizer			*pSizer;
	wxTextCtrl			*pEdit;
	wxGauge				*pProgress;
	CActivityListBox	*pListBox;
	CActivityBoxEvt		*pActivityBoxEvt;
	CActivityEditEvt	*pActivityEditEvt;
    CActivityBox        *m_ParentBox;
	//--- thread stuff ---//
	CThreadController m_ActiveThreadController;
	int m_Progress;
	int m_ProgressType;

friend class CActivityListBox;
};
#if 0
class ActivityDropTarget : public wxTextDropTarget
{
public:
	ActivityDropTarget( CActivityBox *pSList )	{ pActivityCtrl = pSList; pList = pSList->GetListBox(); }

	virtual bool			OnDropText(wxCoord x, wxCoord y, const wxString& text);
	virtual wxDragResult	OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
	void					HighlightSel( wxPoint );

private:
	CActivityBox *pActivityCtrl;
	CActivityListBox *pList;
	int nLastHit;
};
#endif // 0
#endif

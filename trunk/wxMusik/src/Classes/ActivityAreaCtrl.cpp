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
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "ActivityAreaCtrl.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "PlaylistCtrl.h"

BEGIN_EVENT_TABLE(CActivityAreaCtrl, wxSashLayoutWindow)
	EVT_LIST_ITEM_FOCUSED		( -1, CActivityAreaCtrl::OnActivityBoxFocused	)
	EVT_LIST_ITEM_SELECTED		( -1, CActivityAreaCtrl::OnActivityBoxSelected	)
	EVT_LIST_ITEM_DESELECTED	( -1, CActivityAreaCtrl::OnActivityBoxSelected	) // just use the same method for deselection
	EVT_LIST_COL_BEGIN_DRAG		( -1, CActivityAreaCtrl::OnActivityBoxColResize	)
	EVT_LIST_ITEM_ACTIVATED		( -1, CActivityAreaCtrl::OnActivityBoxActivated	)
	EVT_SASH_DRAGGED			( -1, CActivityAreaCtrl::OnSashDragged			)
	EVT_SIZE					(					  CActivityAreaCtrl::OnSize					)
END_EVENT_TABLE()

CActivityAreaCtrl::CActivityAreaCtrl( wxWindow *pParent )
	: wxSashLayoutWindow( pParent, MUSIK_ACTIVITYCTRL, wxPoint( -1, -1 ), wxSize( -1, -1 ), wxTAB_TRAVERSAL|wxNO_BORDER|wxCLIP_CHILDREN | wxSW_3D )
{
	memset(m_ActivityBox,0,sizeof(m_ActivityBox));
	//SetBackgroundColour(WXSYSTEMCOLOUR(wxT("LIGHT STEEL BLUE")));
	m_pPanel = new wxPanel( this, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ), wxNO_BORDER|wxCLIP_CHILDREN|wxTAB_TRAVERSAL );
	pTopSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pPanel->SetSizer( pTopSizer );
	m_Selected = m_bFocused = m_Selecting = false;
	m_UpdatePlaylist = true;
	m_bContentInvalid = true;
	m_pLastSelectedBox = NULL;
	Create();

	wxBoxSizer *pSizer = new wxBoxSizer( wxVERTICAL );
	pSizer->Add( m_pPanel, 1, wxEXPAND , 0 );
	SetSizer( pSizer );
}

CActivityAreaCtrl::~CActivityAreaCtrl()
{

}

bool CActivityAreaCtrl::Create()
{
	Delete();
	for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
	{
		if ( wxGetApp().Prefs.nActBoxType[i] > 0 && m_ActivityBox[i] == NULL )
		{
			m_ActivityBox[i] = new CActivityBox( m_pPanel, MUSIK_ACTIVITYBOX1 + i, wxGetApp().Prefs.nActBoxType[i] );
			pTopSizer->Add( m_ActivityBox[i], 1, wxEXPAND | wxRIGHT, 1 );
		}
	}

	m_pPanel->Layout();
	//	SetSize( pTopSizer->GetMinSize() );

	SetParent( 0, false );

	return true;

}

void CActivityAreaCtrl::Delete()
{
	for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
	{
		if ( m_ActivityBox[i] != NULL )
		{
#if wxVERSION_NUMBER >= 2500
			pTopSizer->Detach( m_ActivityBox[i] );
#else
			pTopSizer->Remove( m_ActivityBox[i] );
#endif
			delete m_ActivityBox[i];
			m_ActivityBox[i] = NULL;
		}
	}
}

void CActivityAreaCtrl::ResetAllContents( bool bUpdatePlaylist )
{
	if(!IsShown())
	{
		m_bContentInvalid = true;
		return;
	}
	m_UpdatePlaylist = bUpdatePlaylist;
	for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
	{
		if ( m_ActivityBox[i] != NULL )	m_ActivityBox[i]->ResetContents();
	}
	m_bContentInvalid = false;
	m_UpdatePlaylist = true;
}

bool CActivityAreaCtrl::Show(bool show /* = TRUE  */)
{
	bool bRes = wxSashLayoutWindow::Show(show);
	if(show && m_bContentInvalid)
	{
		ResetAllContents();
	}
	return bRes;
}
void CActivityAreaCtrl::SetParent( int nID, bool bUpdate )
{
	m_ParentId = nID;
	if ( nID == 0 )
{
		m_ParentBox = NULL;
		return;
	}
	else if ( nID >= MUSIK_ACTIVITYBOX1 && nID <= MUSIK_ACTIVITYBOX4)
	{
		m_ParentBox = m_ActivityBox[nID - MUSIK_ACTIVITYBOX1];
	}
	else
	{
		wxASSERT(false);
	}
	if ( m_ParentBox != NULL && bUpdate )
		UpdateSel( m_ParentBox );
	return;

}

void CActivityAreaCtrl::UpdateSel( CActivityBox *pSelectedBox )
{
	if(pSelectedBox == NULL)
	{
	   pSelectedBox =  m_pLastSelectedBox;
	}
	else
		m_pLastSelectedBox = pSelectedBox;

	if ( !pSelectedBox )
	{
		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
			wxGetApp().Library.GetAllSongs( g_Playlist );
		return;
	}
	//-------------------------------------//
	//--- which box are we?             ---//
	//--- and which are the other ones? ---//
	//-------------------------------------//
	CActivityBox *pOtherBoxes[ActivityBoxesMaxCount-1];
	memset(	pOtherBoxes ,0,sizeof(pOtherBoxes));

	for(size_t i = 0 ; i < ActivityBoxesMaxCount;i++)
	{
		if ( pSelectedBox == m_ActivityBox[i] )
		{	
			size_t k = 0;
			// assign not selected boxes to pOtherBoxes array
			for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
			{
 				if(k == i)
				{
					k++;
				}
				if(k > ActivityBoxesMaxCount-1)
					k = 0;
				pOtherBoxes[j] = m_ActivityBox[k++];
			}
			break;
		}
	}
			
	
	
	//-------------------------------------------------//
	//--- if we're showing unsel, find which is the	---//
	//--- parent and which are children, if there	---//
	//--- is no parent already.						---//
	//-------------------------------------------------//
	if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD )
	{
		if ( GetParentId() == 0 )
			SetParent( pSelectedBox->GetListId(), false );
	}
 
	//---------------------------------------------------//
	//--- if we're hiding unselected entries          ---//
	//--- and reset is clicked or nothing is selected ---//
	//--- reset all the boxes						  ---//
	//---------------------------------------------------//
	if ( ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY ) && ( pSelectedBox->IsSelected( 0 ) || pSelectedBox->GetSelectedItemCount() < 1 ) )
	{
		SetParent( 0, false );
		pSelectedBox->ResetContents();	

		for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		{
			if ( pOtherBoxes[j] != NULL )	
				pOtherBoxes[j]->ResetContents();	
		}
		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
		{
			wxGetApp().Library.GetAllSongs( g_Playlist );
			g_PlaylistBox->Update();
			return;
		}
	}

	//-----------------------------------------------//
	//--- if we're hiding unselected entries      ---//
	//--- and a valid item is clicked, update the ---//
	//--- other controls with the right values    ---//
	//-----------------------------------------------//
	else if ( ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY ) && ( !pSelectedBox->IsSelected( 0 ) && pSelectedBox->GetSelectedItemCount() > 0 ) )
	{
		wxArrayString temp_list;
		if ( ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD && GetParentId() == pSelectedBox->GetListId() ) || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY )
		{
			for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
			{
				if ( pOtherBoxes[j] != NULL )
				{
					pSelectedBox->GetRelatedList( pOtherBoxes[j], temp_list );
					pOtherBoxes[j]->SetContents( temp_list );
				}
			}
		}
	}      

	//-------------------------------------------//
	//--- if we're showing unselected entries ---//
	//--- and no items are selected, unselect ---//
	//--- all the corresponding items		  ---//
	//-------------------------------------------//
	else if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_HIGHLIGHT && pSelectedBox->GetSelectedItemCount() < 1 )
	{
		pSelectedBox->DeselectAll();
		for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		{
			if ( pOtherBoxes[j] != NULL )	pOtherBoxes[j]->DeselectAll();
		}

		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
		{
			wxGetApp().Library.GetAllSongs( g_Playlist );
			g_PlaylistBox->Update();
			return;
		}		
	}

	//-------------------------------------------//
	//--- if we're showing unselected entries ---//
	//--- and 1+ items are selected, select   ---//
	//--- all the corresponding items		  ---//
	//-------------------------------------------//
	else if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_HIGHLIGHT && pSelectedBox->GetSelectedItemCount() > 0 )
	{
		wxArrayString rel;
		wxArrayString all;
		wxListItem item;
		int n;

		for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		{
			if ( pOtherBoxes[j] != NULL )
			{
				pSelectedBox->GetRelatedList( pOtherBoxes[j], rel );
				pOtherBoxes[j]->GetFullList( all );

				if ( all.GetCount() > 0 )
				{
					for ( size_t i = 0; i < rel.GetCount(); i++ )
					{
						n = FindStrInArray( &all, rel.Item( i ) );
						if ( n  > -1 )
						{
							all.RemoveAt( n );
							all.Insert( rel.Item( i ), i );
						}
					}
					pOtherBoxes[j]->SetRelated( rel.GetCount() );
					pOtherBoxes[j]->SetContents( all );
				}
			}
		}
		pSelectedBox->SetRelated( -1 );
		pSelectedBox->Update( false );

	}

	if ( ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY ) && ( pSelectedBox->IsSelected( 0 ) || pSelectedBox->GetSelectedItemCount() < 1 ) )
	{
		g_Playlist.Clear();
		g_PlaylistBox->Update();
	}
	else
		pSelectedBox->SetPlaylist();
}



//-------------------------------------------------------------------------------------------------//
//---											events											---//
//-------------------------------------------------------------------------------------------------//
//---							*IMPORTANT* *IMPORTANT* *IMPORTANT*								---//
//--- when using a virtual wxListCtrl, there are a few selection problems. specifically,		---//
//--- if the user holds shift to select, the selection will never get updated if the			---//
//--- event EVT_LIST_ITEM_SELECTED is used. so, instead we use EVT_LIST_ITEM_FOCUSED...			---//
//--- EVT_LIST_ITEM_SELECTED is not fired on virtual list box if multi selection is done		---//
//----by the user.																				---//			
//--- On Windows EVT_LIST_ITEM_FOCUSED is not fired if the item already has the focus.			---//
//--- So in windows, we cannot use  EVT_LIST_ITEM_FOCUSED alone.								---//
//--- Order of  EVT_LIST_ITEM_SELECTED and EVT_LIST_ITEM_FOCUSED is different on linux and win. ---//
//--- we have to assure only one is execute, because if they are both called, the selection		---//
//--- event will be called twice. no good. thats why there are three events that do the same	---//
//--- thing.																					---//
//--- to save the #ifdef code on linux and win is the same, would not be						---//
//--- neccessary but doesnt harm.																---//
//-------------------------------------------------------------------------------------------------//

void CActivityAreaCtrl::OnActivityBoxFocused( wxListEvent& event )
{
	if( m_Selected )
	{
		m_Selected = false;
		return;
	}

	wxListEvent ev(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, event.GetId() );
	m_bFocused = true;
	::wxPostEvent( this, ev );
}

void CActivityAreaCtrl::OnActivityBoxSelected( wxListEvent& event )
{
	if( m_bFocused )
	{
		m_bFocused = false;
		return;
	}
	wxListEvent ev(wxEVT_COMMAND_LIST_ITEM_ACTIVATED,event.GetId());
	m_Selected = true;
	::wxPostEvent( this, ev );
}

CActivityBox* CActivityAreaCtrl::GetActivityBox(EMUSIK_ACTIVITY_TYPE eType)
{
	for(size_t i = 0; i < ActivityBoxesMaxCount; i++)
	{
		if( m_ActivityBox[i] && m_ActivityBox[i]->GetActivityType() == eType )
		{
			return m_ActivityBox[i];
		}
	}
	return NULL;
}

//----------------------------//
//--- item activated event ---//
//----------------------------//
void CActivityAreaCtrl::OnActivityBoxActivated	( wxListEvent& event)
{	
	if ( !m_Selecting && m_UpdatePlaylist )
	{
		m_Selecting = true;
		UpdateSel( m_ActivityBox[event.GetId() - MUSIK_ACTIVITYBOX1] );
		m_Selected = m_bFocused = m_Selecting = false;
	}
}

void CActivityAreaCtrl::OnSashDragged	(wxSashEvent & ev)
{
	wxGetApp().Prefs.nActivityCtrlHeight = ev.GetDragRect().height;
	SetDefaultSize(wxSize( 1000, wxGetApp().Prefs.nActivityCtrlHeight));
	ev.Skip();
}
void CActivityAreaCtrl::OnSize( wxSizeEvent& event )
{
	wxSashLayoutWindow::OnSize(event);
	m_pPanel->Layout();
	
}

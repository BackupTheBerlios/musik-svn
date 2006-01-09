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
#include "myprec.h"

#include "ActivityAreaCtrl.h"

//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"

#include "PlaylistCtrl.h"

BEGIN_EVENT_TABLE(CActivityAreaCtrl, wxSashLayoutWindow)
	EVT_LISTSEL_CHANGED_COMMAND ( wxID_ANY,	CActivityAreaCtrl::OnActivityBoxSelChanged	)
	EVT_LIST_ITEM_ACTIVATED		( wxID_ANY, CActivityAreaCtrl::OnActivityBoxActivated	)
	EVT_SASH_DRAGGED			( wxID_ANY, CActivityAreaCtrl::OnSashDragged			)
	EVT_SIZE					( CActivityAreaCtrl::OnSize )
END_EVENT_TABLE()

CActivityAreaCtrl::CActivityAreaCtrl( wxWindow *pParent )
	: wxSashLayoutWindow( pParent, MUSIK_ACTIVITYCTRL, wxPoint( -1, -1 ), wxSize( -1, -1 ), wxTAB_TRAVERSAL|wxNO_BORDER|wxCLIP_CHILDREN | wxSW_3D )
{
	memset(m_ActivityBox,0,sizeof(m_ActivityBox));
	//SetBackgroundColour(WXSYSTEMCOLOUR(wxT("LIGHT STEEL BLUE")));
	m_pPanel = new wxPanel( this, -1, wxPoint( -1, -1 ), wxSize( -1, -1 ), wxNO_BORDER|wxCLIP_CHILDREN|wxTAB_TRAVERSAL );
	pTopSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pPanel->SetSizer( pTopSizer );
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

bool CActivityAreaCtrl::ReCreate()
{
    Delete();
    Create();
    Show();
    return true;
}
bool CActivityAreaCtrl::Create()
{
	for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
	{
		if ( wxGetApp().Prefs.nActBoxType[i] >= 0 && m_ActivityBox[i] == NULL )
		{
			m_ActivityBox[i] = new CActivityBox( m_pPanel, MUSIK_ACTIVITYBOX1 + i, wxGetApp().Prefs.nActBoxType[i] );
			pTopSizer->Add( m_ActivityBox[i], 1, wxEXPAND | wxRIGHT, 1 );
		}
	}

	m_pPanel->Layout();
	//	SetSize( pTopSizer->GetMinSize() );

	SetParent( NULL, false );
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
    m_bContentInvalid = true;
}

void CActivityAreaCtrl::ReloadAllContents(  )
{
    if(!IsShown())
    {
        m_bContentInvalid = true;
        return;
    }
    for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
    {
        if ( m_ActivityBox[i] != NULL )	m_ActivityBox[i]->ReloadContents();
    }
    m_bContentInvalid = false;
}

void CActivityAreaCtrl::ResetAllContents(  )
{
	if(!IsShown())
	{
		m_bContentInvalid = true;
		return;
	}
	for(size_t i = 0; i < ActivityBoxesMaxCount;i++)
	{
		if ( m_ActivityBox[i] != NULL )	m_ActivityBox[i]->ResetContents(RCM_EnsureVisibilityOfCurrentTopItem);
	}
	m_bContentInvalid = false;
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
void CActivityAreaCtrl::SetParent( CActivityBox* pBox, bool bUpdate )
{
	if ( pBox == NULL )
    {
		m_ParentBox = NULL;
		return;
	}
	else 
	{
		m_ParentBox = pBox;
	}
	if ( m_ParentBox != NULL && bUpdate )
		UpdateSel( m_ParentBox );
	return;
}
class ReentrantProtect
{
public:
    ReentrantProtect(int & cnt)
        :m_RepCnt(cnt)
    {
        m_RepCnt++;
    }
    bool Active()
    {
        return m_RepCnt > 1;
    }
    ~ReentrantProtect()
    {
        m_RepCnt--;
    }
private:
    int & m_RepCnt;       
};

#define RETURN_ON_REENTRY \
    static int repcnt_xx = 0;          \
    ReentrantProtect rep_xx(repcnt_xx);   \
    if(rep_xx.Active())                \
        return;                     

void CActivityAreaCtrl::UpdateSel( CActivityBox *pSelectedBox ,bool bForceShowAll)
{
    RETURN_ON_REENTRY;
    wxBusyCursor  busycursor;
	if(pSelectedBox == NULL)
	{
	   pSelectedBox =  m_pLastSelectedBox;
	}
	else
		m_pLastSelectedBox = pSelectedBox;

	if ( !pSelectedBox )
	{
		if ( wxGetApp().Prefs.bShowAllSongs == 1 )
		{
			wxGetApp().Library.GetAllSongs( g_thePlaylist );
			g_PlaylistBox->SetPlaylist( &g_thePlaylist);
		}
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
                        if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_NORMAL )
                        {
                                for(size_t j = i+1 ; j < ActivityBoxesMaxCount; j++)
                                {
                                        pOtherBoxes[k] = m_ActivityBox[j];
                                        k++;
                                }
                        }
                        else
                        {
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
                        }
			break;
		}
	}
			
	
	
	//-------------------------------------------------//
	//--- if we're showing unsel, find which is the	---//
	//--- parent and which are children, if there	---//
	//--- is no parent already.						---//
	//-------------------------------------------------//
	if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_NORMAL )
	{
		if(GetParentBox() == NULL)
			SetParent( pSelectedBox , false );
	}
    if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_NORMAL )
    {
	    //---------------------------------------------------//
	    //--- if we're hiding unselected entries          ---//
	    //--- and reset is clicked or nothing is selected ---//
	    //--- reset all the boxes						  ---//
	    //---------------------------------------------------//
	    if ( pSelectedBox->IsSelected( 0 ) || pSelectedBox->GetSelectedItemCount() == 0 )
	    {
		    SetParent( NULL, false );
		    pSelectedBox->ResetContents(RCM_EnsureVisibilityOfCurrentTopItem); // ensure visibility of top item before reset	

		    for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		    {
			    if ( pOtherBoxes[j] != NULL )	
				    pOtherBoxes[j]->ResetContents();	
		    }
		    if ( wxGetApp().Prefs.bShowAllSongs == 1)
		    {
			    wxGetApp().Library.GetAllSongs( g_thePlaylist );
			    g_PlaylistBox->SetPlaylist( &g_thePlaylist);
			    return;
		    }
            else if ( !bForceShowAll &&(pSelectedBox->IsSelected( 0 ) || pSelectedBox->GetSelectedItemCount() == 0 ))
            {
                g_thePlaylist.Clear();
                g_PlaylistBox->SetPlaylist(&g_thePlaylist);
                return;
            }
	    }

	    //-----------------------------------------------//
	    //--- if we're hiding unselected entries      ---//
	    //--- and a valid item is clicked, update the ---//
	    //--- other controls with the right values    ---//
	    //-----------------------------------------------//
	    else if ( !pSelectedBox->IsSelected( 0 ) && pSelectedBox->GetSelectedItemCount() > 0 )
	    {
            if(bForceShowAll)
            {
                SetParent( pSelectedBox , false );
                pSelectedBox->ResetContents(RCM_PreserveSelectedItems); // ensure visiblility of top item before reset
            }
		    wxArrayString temp_list;
		    if ( ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_STANDARD && GetParentBox() == pSelectedBox ) 
                || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_NORMAL
                || wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_SLOPPY )
		    {
			    for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
			    {
				    if ( pOtherBoxes[j] != NULL )
				    {
					    pOtherBoxes[j]->GetRelatedList(pSelectedBox , temp_list );
					    pOtherBoxes[j]->SetContents( temp_list );
				    }
			    }
		    }
	    }  
        pSelectedBox->SetPlaylist();
    }
	else if ( wxGetApp().Prefs.eSelStyle == MUSIK_SELECTION_TYPE_HIGHLIGHT)
    {
        //-------------------------------------------//
        //--- if we're showing unselected entries ---//
        //--- and no items are selected, unselect ---//
        //--- all the corresponding items		  ---//
        //-------------------------------------------//
        if(pSelectedBox->GetSelectedItemCount() == 0)
	    {
		    pSelectedBox->SelectNone();
		    for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		    {
			    if ( pOtherBoxes[j] != NULL )	pOtherBoxes[j]->SelectNone();
		    }

		    if ( wxGetApp().Prefs.bShowAllSongs == 1 )
		    {
			    wxGetApp().Library.GetAllSongs( g_thePlaylist );
			    g_PlaylistBox->SetPlaylist(&g_thePlaylist);
			    return;
		    }		
	    }

	    else 
	    {
            //----------------------------------------------------//
            //--- if we're showing unselected entries          ---//
            //--- and one or more items are selected, select   ---//
            //--- all the corresponding items		           ---//
            //----------------------------------------------------// 
		    wxArrayString rel;
		    wxArrayString all;
		    wxListItem item;
		    int n;

		    for(size_t j = 0 ; j < WXSIZEOF(pOtherBoxes);j++)
		    {
			    if ( pOtherBoxes[j] != NULL )
			    {
				    pOtherBoxes[j]->GetRelatedList( pSelectedBox, rel );
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
            pSelectedBox->SetPlaylist();

	    }
    }		
}

CActivityBox* CActivityAreaCtrl::GetActivityBox(size_t nIndex)
{
    return m_ActivityBox[nIndex];
}
CActivityBox* CActivityAreaCtrl::GetActivityBox(PlaylistColumn::eId ColId)
{
	for(size_t i = 0; i < ActivityBoxesMaxCount; i++)
	{
		if( m_ActivityBox[i] && m_ActivityBox[i]->ColId() == ColId )
		{
			return m_ActivityBox[i];
		}
	}
	return NULL;
}

size_t CActivityAreaCtrl::GetActivityBoxCount()
{
    for(size_t i = 0; i < ActivityBoxesMaxCount; i++)
    {
        if( m_ActivityBox[i] == NULL)
        {
            return i;
        }
    }
    return ActivityBoxesMaxCount;
}
//----------------------------//
//--- item activated event ---//
//----------------------------//
void CActivityAreaCtrl::OnActivityBoxActivated(  wxListEvent& event )
{
    UpdateSel( m_ActivityBox[event.GetId() - MUSIK_ACTIVITYBOX1] , true ); // true means force show all
}
void CActivityAreaCtrl::OnActivityBoxSelChanged	(  wxCommandEvent& event )
{	
	UpdateSel( m_ActivityBox[event.GetId() - MUSIK_ACTIVITYBOX1] );
    event.Skip();
}

void CActivityAreaCtrl::OnSashDragged	(wxSashEvent & ev)
{
	wxGetApp().Prefs.nActivityCtrlHeight = wxMax(ev.GetDragRect().height,50);
	SetDefaultSize(wxSize( 1000, wxGetApp().Prefs.nActivityCtrlHeight));
	ev.Skip();
}
void CActivityAreaCtrl::OnSize( wxSizeEvent& event )
{
	wxSashLayoutWindow::OnSize(event);
	m_pPanel->Layout();
	
}

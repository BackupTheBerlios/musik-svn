/*
*  PictureBox.cpp
*
*  The "Picture Box" control
*	This is the control visible on the lower left side of the main frame.
*	It's main function is to display a picture.
*
*  Copyright (c) 2004  Gunnar Roth (gunnar.roth@gmx.de)
*	Contributors: Casey Langen,Simon Windmill, Dustin Carter, Wade Brainerd
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
//--- For compilers that support precompilation, includes "wx/wx.h". ---//
#include "wx/wxprec.h"

#include "PictureBox.h"
#include <wx/dir.h>
//--- globals ---//
#include "../MusikGlobals.h"
#include "../MusikUtils.h"


BEGIN_EVENT_TABLE(CPictureBox, wxPanel)
	EVT_PAINT(CPictureBox::OnPaint)
	EVT_ERASE_BACKGROUND( CPictureBox::OnEraseBackground )
	EVT_IDLE(CPictureBox::OnIdle)
	EVT_LEFT_DOWN(CPictureBox::OnLeftDown)
	EVT_TIMER(-1, CPictureBox::OnTimer)
END_EVENT_TABLE()

CPictureBox::CPictureBox( wxWindow *parent )
: wxPanel( parent, MUSIK_PICTURE_BOX, wxPoint( -1, -1 ), wxSize( -1, -1 ),  wxCLIP_CHILDREN |wxSW_3D|wxTAB_TRAVERSAL )
,m_Timer(this)
{
	m_bDefaultImageFailed = false;
	Show(false);
}
void CPictureBox::OnIdle(wxIdleEvent & event)
{
	if(IsShown() == false || (GetParent() && (GetParent()->IsShown() == false)))
	{
		event.Skip();
		return;
	}
	int nSel = g_PlaylistBox->PlaylistCtrl().GetNextItem( -1, wxLIST_NEXT_ALL , wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
	if(g_Playlist.GetCount())
	{// 
		int i = wxMax(nSel,0);
		if(m_currSongPath != g_Playlist[i].MetaData.Filename)
		{// select song has changed
			m_currSongPath = g_Playlist[i].MetaData.Filename;
			TryToLoadImage(m_currSongPath);
		//	bAllowShowPlaying =false;
			m_Timer.Start(3000,wxTIMER_ONE_SHOT);
			return;
		}	
	}
	if( wxGetApp().Player.IsPlaying())
	{// we try to show an image for the currently playing song
		if(m_currPlayingSongPath != wxGetApp().Player.GetCurrentFilename())
		{// song name has changed so we load new image
			m_currPlayingSongPath = wxGetApp().Player.GetCurrentFilename();
			TryToLoadImage(m_currPlayingSongPath);
			return;
		}
		return;
	}
	if(!g_Playlist.GetCount())
	{
		if(m_image	!= m_DefImage)
		{
			m_image	= m_DefImage;
			Refresh();
			return;
		}
	}
	event.Skip();
}
void CPictureBox::OnLeftDown(wxMouseEvent & event)
{
	if(m_img_files.GetCount() <= 1)
		return;
	if(event.ShiftDown())
	{
		if(m_curr_img_file == 0)
			m_curr_img_file = m_img_files.GetCount();
		else
			--m_curr_img_file;
	}
	else
	{
		if(m_curr_img_file == m_img_files.GetCount()-1)
			m_curr_img_file = 0;
		else
			++m_curr_img_file;
	}
	m_image.LoadFile(m_img_files[m_curr_img_file]);
	Refresh();
}
class wxDirTraverserImages : public wxDirTraverser
{
public:
	wxDirTraverserImages(wxArrayString& files) : m_files(files) { }

	virtual wxDirTraverseResult OnFile(const wxString& filename)
	{
		wxString ext(filename.Right(4));
		ext.MakeLower();
		if ( ext == wxT( ".gif" ) || ext == wxT( ".jpg" )|| ext == wxT( ".bmp" )||ext == wxT( ".png" )) 
		{
			m_files.push_back(filename);
		}
		return wxDIR_CONTINUE;
	}

	virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname))
	{
		return wxDIR_STOP;
	}

private:
	wxArrayString& m_files;

	DECLARE_NO_COPY_CLASS(wxDirTraverserImages)
};

void CPictureBox::TryToLoadImage(const CSongPath &sSongPath)
{
	if(sSongPath.IsUrl())
	{
		m_image	= m_DefImage;
		return;
	}
	m_img_files.Clear();
	wxDir d(sSongPath.GetPath());
	wxDirTraverserImages t(m_img_files);
	d.Traverse(t);
	for(m_curr_img_file = 0; m_curr_img_file < m_img_files.GetCount();m_curr_img_file++)
	{
		wxString s(m_img_files[m_curr_img_file]);
		if(s.Lower().Contains(wxT("front")))
		{
			if(m_image.LoadFile(s))
			{
				Refresh();
				return;
			}
		}
	
	}
	m_curr_img_file = 0;
	if(m_img_files.GetCount())
		m_image.LoadFile(m_img_files[m_curr_img_file]);
	else
		m_image	= m_DefImage;

	if(!m_image.Ok())
		m_image	= m_DefImage;
	Refresh();
}

void CPictureBox::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);

	if(m_DefImage.Ok() == false && m_bDefaultImageFailed == false)
	{
		m_image = m_DefImage = wxImage(MusikGetStaticDataPath() + wxT("musik.png"), wxBITMAP_TYPE_PNG);
		if(!m_DefImage.Ok())
		{
			m_bDefaultImageFailed = true;
		}
	}
	wxCoord width,height;
	dc.GetSize(&width,&height);
	// Now  declare the Clipping Region which is
	// what needs to be repainted
	wxRegion MyRegion(0,0,width,height); 
	wxRect cliprect;
	dc.GetClippingBox(cliprect); 
	MyRegion.Subtract(cliprect);

	const int offset= 5;
	width -= 2*offset;
	height -= 2*offset;
	int bmpwidth = m_image.GetWidth(),bmpheight = m_image.GetHeight();
	if((double)width/(double)height > (double)bmpwidth/(double)bmpheight)
	{
		if(bmpwidth > bmpheight)
		{
			bmpwidth = width;
			bmpheight = (int)(((double)height * bmpwidth)/((double)width) + 0.5);			
		}
		else
		{
			bmpheight = height;
			bmpwidth = (int)(((double)width * bmpheight)/((double)height) + 0.5);
			
		}
	}
	else
	{
		if(bmpheight > bmpwidth)
		{
			bmpheight = height;
			bmpwidth = (int)(((double)width * bmpheight)/((double)height) + 0.5);
		}
		else
		{
			bmpwidth = width;
			bmpheight = (int)(((double)height * bmpwidth)/((double)width) + 0.5);
		}
	}
	m_image.ConvertAlphaToMask();
	wxBitmap bmp = wxBitmap(m_image.Scale(bmpwidth, bmpheight));
	wxCoord x = abs(bmpwidth - width)/2 + offset;
	wxCoord y = abs(bmpheight - height)/2 + offset ;
	dc.DrawBitmap(bmp,x,y,true);
	wxColour BGColor =  GetBackgroundColour();
	wxBrush MyBrush(BGColor ,wxSOLID);
	dc.SetBackground(MyBrush);
	wxRegion regionBmp(bmp);
	regionBmp.Offset(x,y);
	MyRegion.Subtract(regionBmp);
	// now destroy the old clipping region
	dc.DestroyClippingRegion();
	//and set the new one
	dc.SetClippingRegion(MyRegion);
	dc.Clear();
}

void CPictureBox::OnTimer(wxTimerEvent& )
{
	m_currPlayingSongPath = wxT("");
	wxWakeUpIdle();
}


CPictureBox::~CPictureBox()
{

}

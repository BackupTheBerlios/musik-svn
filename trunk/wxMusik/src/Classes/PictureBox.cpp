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
#include "myprec.h"

#include "PictureBox.h"
#include <wx/dir.h>
//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"
#include "Classes/PlaylistCtrl.h" //TODO: remove the dependancy
#include "Library/TagLibInfo.h"

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
	int nSel = g_PlaylistBox->PlaylistCtrl().GetCurrentSelection();
	if(nSel > -1)
	{// 
        std::auto_ptr<CMusikSong> pSong = g_PlaylistBox->PlaylistCtrl().Playlist()[nSel].Song();

		if(m_currSongPath != pSong->MetaData.Filename)
		{// select song has changed
			m_currSongPath = pSong->MetaData.Filename;
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
	if(nSel == -1)
	{
#if !wxCHECK_VERSION(2,7,2)
		if(m_image != m_DefImage)
#else
		if(m_image.IsSameAs(m_DefImage))
#endif
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
    wxLogNull lognull; // disable logging in this scope
	m_img_files.Clear();
    if(!LoadImageFromFile(sSongPath.GetFullPath()))
    {
        if(!LoadImageFromDirectory(sSongPath.GetPath()))
        {
            m_image	= m_DefImage;
        }
    }
	if(!m_image.Ok())
		m_image	= m_DefImage;
	Refresh();
}
bool CPictureBox::LoadImageFromFile( const wxString & sFilename )
{
    return CTagLibInfo::LoadImage(sFilename, m_image);
}
bool CPictureBox::LoadImageFromDirectory(const wxString &sDirPath)
{
    wxDir d(sDirPath);
    wxDirTraverserImages t(m_img_files);
    d.Traverse(t);
    for(m_curr_img_file = 0; m_curr_img_file < m_img_files.GetCount();m_curr_img_file++)
    {
        wxString s(m_img_files[m_curr_img_file]);
        if(s.Lower().Contains(wxT("front")))
        {
            if(m_image.LoadFile(s))
            {
                return true;
            }
        }

    }
    m_curr_img_file = 0;
    if(m_img_files.GetCount())
        m_image.LoadFile(m_img_files[m_curr_img_file]);
    else
        return false;
    return true;
}
void CPictureBox::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);

	if(m_DefImage.Ok() == false && m_bDefaultImageFailed == false)
	{
        wxLogNull lognull; // disable logging in this scope
		m_image = m_DefImage = wxImage(MusikGetStaticDataPath() + wxT("wxMusik.jpg"), wxBITMAP_TYPE_ANY);
		if(!m_DefImage.Ok())
		{
			m_bDefaultImageFailed = true;
		}
	}
	wxCoord width,height;
	dc.GetSize(&width,&height);
	// we use a clipping region to paint border and picture flickerfree.
#ifdef __WXMAC__
// in wxWidgets 2.7.2 the clipping stuff for osx is broken,SetClippingRegion seems to be ignored,
// and as osx uses double buffering on system level, we dont really need the
// clipping stuff here.
        wxColour BGColor =  GetBackgroundColour();
        wxBrush MyBrush(BGColor ,wxSOLID);
        dc.SetBackground(MyBrush);
        dc.Clear();
#else
	// Now  declare the Clipping Region which is
	// what needs to be repainted
	wxRegion MyRegion(0,0,width,height); 
	wxRect cliprect;
	dc.GetClippingBox(cliprect); 
	MyRegion.Subtract(cliprect);
#endif
    if(m_image.Ok())
    {
	    const int offset= 1;
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
	    wxBitmap bmp = 
#if !wxCHECK_VERSION(2,7,2)
                 		wxBitmap(m_image.Scale(bmpwidth,bmpheight));
#else
		 		wxBitmap(m_image.Scale(bmpwidth,bmpheight,wxIMAGE_QUALITY_HIGH));
#endif

	    wxCoord x = abs(bmpwidth - width)/2 + offset;
	    wxCoord y = abs(bmpheight - height)/2 + offset ;
	    dc.DrawBitmap(bmp,x,y,true);
#ifndef __WXMAC__
	    wxRegion regionBmp(bmp);
	    regionBmp.Offset(x,y);
	    MyRegion.Subtract(regionBmp);
	    // now destroy the old clipping region
	    dc.DestroyClippingRegion();
	    //and set the new one
	    dc.SetClippingRegion(MyRegion);
#endif
    }
#ifndef __WXMAC__
    wxColour BGColor =  GetBackgroundColour();
    wxBrush MyBrush(BGColor ,wxSOLID);
    dc.SetBackground(MyBrush);
    dc.Clear();
#endif
}

void CPictureBox::OnTimer(wxTimerEvent& )
{
	m_currPlayingSongPath = wxT("");
	wxWakeUpIdle();
}


CPictureBox::~CPictureBox()
{

}

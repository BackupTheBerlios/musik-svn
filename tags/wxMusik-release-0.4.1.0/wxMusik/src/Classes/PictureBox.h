/*
*  PictureBox.h
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

#ifndef MUSIK_PICTURE_BOX_H
#define MUSIK_PICTURE_BOX_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif 
#include <wx/image.h>
#include <wx/laywin.h>
#include "../MusikUtils.h"

class CPictureBox : public wxPanel
{
public:
	CPictureBox( wxWindow *parent );
	~CPictureBox();

	void OnEraseBackground( wxEraseEvent&  ){}
	void OnPaint(wxPaintEvent &event);
	void OnIdle(wxIdleEvent & event);
	void OnLeftDown(wxMouseEvent & event);
	void OnTimer(wxTimerEvent& event);
	DECLARE_EVENT_TABLE()
protected:
	void TryToLoadImage(const CSongPath &sSongPath);
private:
	wxImage m_DefImage;
	wxImage m_image;
	CSongPath m_currSongPath;
	CSongPath m_currPlayingSongPath;
	wxArrayString m_img_files;
	size_t m_curr_img_file;
	bool m_bDefaultImageFailed;
	wxTimer		m_Timer;
};


#endif

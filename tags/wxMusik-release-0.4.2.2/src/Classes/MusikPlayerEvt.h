/*
*  MusikPlayerEvt.h
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*	
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef _WX_MUSIKPLAYEREVT_H_
#define _WX_MUSIKPLAYEREVT_H_

#include "wx/event.h"
#include "wx/window.h"

// ----------------------------------------------------------------------------
// MusikPlayerEvt: used by MusikPlayer
// ----------------------------------------------------------------------------
class CMusikPlayer;

class MusikPlayerEvent : public wxCommandEvent
{
public:
    MusikPlayerEvent() { }
    MusikPlayerEvent(CMusikPlayer *pMP, wxEventType type);

    MusikPlayerEvent(const MusikPlayerEvent& event)
        : wxCommandEvent(event)
    {
    }

    virtual wxEvent *Clone() const { return new MusikPlayerEvent(*this); }


    CMusikPlayer & MusikPlayer();
private:

    DECLARE_DYNAMIC_CLASS_NO_ASSIGN(MusikPlayerEvent)
};

// ----------------------------------------------------------------------------
// event types and macros for handling them
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_MUSIKPLAYER_SONG_CHANGED, 1101)
DECLARE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_START, 1101)
DECLARE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_STOP, 1101)
DECLARE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_PAUSE, 1101)
DECLARE_EVENT_TYPE(wxEVT_MUSIKPLAYER_PLAY_RESUME, 1101)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*MusikPlayerEventFunction)(MusikPlayerEvent&);

#define MusikPlayerEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(MusikPlayerEventFunction, &func)

#define EVT_MUSIKPLAYER_SONG_CHANGED(fn) \
    wx__DECLARE_EVT0(wxEVT_MUSIKPLAYER_SONG_CHANGED, MusikPlayerEventHandler(fn))
#define EVT_MUSIKPLAYER_PLAY_START(fn) \
    wx__DECLARE_EVT0(EVT_MUSIKPLAYER_PLAY_START, MusikPlayerEventHandler(fn))
#define EVT_MUSIKPLAYER_PLAY_STOP(fn) \
    wx__DECLARE_EVT0(wxEVT_MUSIKPLAYER_PLAY_STOP, MusikPlayerEventHandler(fn))
#define EVT_MUSIKPLAYER_PLAY_PAUSE(fn) \
    wx__DECLARE_EVT0(wxEVT_MUSIKPLAYER_PLAY_PAUSE, MusikPlayerEventHandler(fn))
#define EVT_MUSIKPLAYER_PLAY_RESUME(fn) \
    wx__DECLARE_EVT0(wxEVT_MUSIKPLAYER_PLAY_RESUME, MusikPlayerEventHandler(fn))


#endif // _WX_MUSIKPLAYEREVT_H_

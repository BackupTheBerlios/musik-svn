#ifndef STREAMINGBUFFERPANEL_H
#define STREAMINGBUFFERPANEL_H

#include "PrefPanel.h"

class StreamingBufferPanel : public PrefPanel
{
public:
    StreamingBufferPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Buffer" );
    }
protected:
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

private:
    wxSpinCtrl* scStreamingBufferSize;
    wxSpinCtrl* scStreamingPreBufferPercent;
    wxSpinCtrl* scStreamingReBufferPercent;

};
#endif
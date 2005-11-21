#ifndef SOUNDCROSSFADERPANEL_H
#define SOUNDCROSSFADERPANEL_H

#include "PrefPanel.h"

class SoundCrossfaderPanel : public PrefPanel
{
public:
    SoundCrossfaderPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Crossfader" );
    }

protected:
    virtual void DoLoadPrefs();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

private:
    wxTextCtrl	*tcDuration;
    wxTextCtrl	*tcSeekDuration;
    wxTextCtrl	*tcPauseResumeDuration;
    wxTextCtrl	*tcStopDuration;
    wxTextCtrl	*tcExitDuration;

};

#endif

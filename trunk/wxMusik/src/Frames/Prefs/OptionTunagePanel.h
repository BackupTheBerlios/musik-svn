#ifndef OPTIONTUNAGEPANEL_H
#define OPTIONTUNAGEPANEL_H

#include "PrefPanel.h"

class OptionTunagePanel : public PrefPanel
{
public:
    OptionTunagePanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Tunage" );
    }

protected:
    wxSizer *CreateControls();

};
#endif
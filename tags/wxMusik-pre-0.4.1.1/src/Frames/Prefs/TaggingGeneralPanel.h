#ifndef TAGGINGGENERALPANEL_H
#define TAGGINGGENERALPANEL_H

#include "PrefPanel.h"

class TaggingGeneralPanel : public PrefPanel
{
public:
    TaggingGeneralPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "General" );
    }

protected:
    wxSizer *CreateControls();

};
#endif
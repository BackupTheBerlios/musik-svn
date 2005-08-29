#ifndef TAGGINGAUTORENAMEPANEL_H
#define TAGGINGAUTORENAMEPANEL_H

#include "PrefPanel.h"

class TaggingAutoRenamePanel : public PrefPanel
{
public:
    TaggingAutoRenamePanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Auto Rename" );
    }

protected:
    wxSizer *CreateControls();
};
#endif

#ifndef OPTIONAUTODJPANEL_H
#define OPTIONAUTODJPANEL_H

#include "PrefPanel.h"

class OptionAutoDJPanel : public PrefPanel
{
public:
    OptionAutoDJPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return wxString(_("Shuffle")) +wxT("/") + _( "Auto DJ" );
    }

protected:
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();
private:
    wxTextCtrl*	tcAutoDjFilter;

};
#endif

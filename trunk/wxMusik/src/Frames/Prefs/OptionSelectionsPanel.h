#ifndef OPTIONSELECTIONSPANEL_H
#define OPTIONSELECTIONSPANEL_H

#include "PrefPanel.h"
#include "Classes/ActivityBox.h"


class OptionSelectionsPanel : public PrefPanel
{
public:
    OptionSelectionsPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Selections" );
    }

protected:
    virtual void DoLoadPrefs();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

private:
    wxComboBox	*cmbActivityBoxes[ActivityBoxesMaxCount];
    wxComboBox	*cmbSelStyle;
};
#endif

#ifndef  OPTIONFILEASSOCPANEL_H
#define OPTIONFILEASSOCPANEL_H

#include "PrefPanel.h"

class OptionFileAssocPanel : public PrefPanel
{
public:
    OptionFileAssocPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "File Associations" );
    }

protected:
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

    void DoFileAssociations();
private:
    wxCheckListBox *chklbFileAssocs;

};
#endif


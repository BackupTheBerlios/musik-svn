#ifndef PREFPANEL_H
#define PREFPANEL_H

#include "PrefUtil.h"

class PrefPanel : public wxPanel
{
public:
    PrefPanel(wxWindow *parent )
        :wxPanel(parent)
    {
    }
    bool SavePrefs()
    {
        if(!Validate() )
            return false;
        if(!TransferDataFromWindow())
            return false;
        return DoSavePrefs();
    }

    virtual wxString Name() = 0;
    DECLARE_EVENT_TABLE()
protected:
    void LoadPrefs()
    {
        TransferDataToWindow();
        DoLoadPrefs();
    }
    virtual bool DoSavePrefs(){ return true;}
    virtual void DoLoadPrefs(){ }
};

#define IMPLEMENT_PREFPANEL_CONSTRUCTOR(panel) \
        panel::panel(wxWindow *parent)     \
        :PrefPanel( parent)                    \
        {                                      \
            SetSizer(CreateControls());        \
            LoadPrefs();                       \
        }

#endif

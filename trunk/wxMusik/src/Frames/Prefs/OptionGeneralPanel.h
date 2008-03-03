#ifndef OPTIONGENERALPANEL_H
#define OPTIONGENERALPANEL_H
#include <wx/clrpicker.h>
#include "PrefPanel.h"

class OptionGeneralPanel : public PrefPanel
{
public:
    OptionGeneralPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "General" );
    }

protected:
    virtual void DoLoadPrefs();
    virtual void AfterDataTransferredFromWindow();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

    DECLARE_EVENT_TABLE()

private:
    struct DbChacheSizeSink : public IOnValueChangeByValidator
    {
        virtual void OnValueChange(wxGenericValidator *pV,wxWindowBase *pW);  
    };

 
    DbChacheSizeSink snkDBCacheSize;
};
#endif


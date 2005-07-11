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
    virtual void AfterDataTransferredFromWindow();
    wxSizer *CreateControls();

private:

    struct NetBufferSettingChangedSink : public IOnValueChangeByValidator
    {
        NetBufferSettingChangedSink()
        {
            m_bNetBufferSettingChanged = false;
        }
       
        virtual void OnValueChange(wxGenericValidator *pV,wxWindowBase *pW)
        {
            m_bNetBufferSettingChanged = true;
        }

         bool m_bNetBufferSettingChanged;
    };

    NetBufferSettingChangedSink snkNBSC;
    
};
#endif

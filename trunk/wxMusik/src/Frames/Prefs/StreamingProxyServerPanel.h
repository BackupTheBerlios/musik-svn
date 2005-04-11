#ifndef STREAMINGPROXYSERVERPANEL_H
#define STREAMINGPROXYSERVERPANEL_H

#include "PrefPanel.h"

class StreamingProxyServerPanel : public PrefPanel
{
public:
    StreamingProxyServerPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Proxy server" );
    }

protected:
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

private:
    wxCheckBox* chkUseProxyServer;
    wxTextCtrl*	tcProxyServer;		
    wxTextCtrl* tcProxyServerPort;	
    wxTextCtrl* tcProxyServerUser;	
    wxTextCtrl* tcProxyServerPassword;	

};
#endif

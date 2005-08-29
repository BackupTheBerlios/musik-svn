#ifndef WEBSERVERPANEL_H
#define WEBSERVERPANEL_H

#include "PrefPanel.h"

class WebServerPanel : public PrefPanel
{
public:
    WebServerPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Internal Webserver" );
    }

protected:
    virtual void AfterDataTransferredFromWindow();
    wxSizer *CreateControls();

private:
    wxCheckBox* chkUseProxyServer;
    wxTextCtrl*	tcProxyServer;		
    wxTextCtrl* tcProxyServerPort;	
    wxTextCtrl* tcProxyServerUser;	
    wxTextCtrl* tcProxyServerPassword;	

};
#endif

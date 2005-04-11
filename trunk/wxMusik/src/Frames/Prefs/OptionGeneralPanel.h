#ifndef OPTIONGENERALPANEL_H
#define OPTIONGENERALPANEL_H

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
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

    void OnClickColour		( wxCommandEvent& event );
    DECLARE_EVENT_TABLE()
private:
    wxCheckBox	*chkSortArtistWithoutPrefix;
    wxCheckBox	*chkPlaylistStripes;
    wxButton	*btnPlaylistStripeColour;
    wxCheckBox	*chkActivityBoxStripes;
    wxButton	*btnActivityStripeColour;
    wxCheckBox	*chkSourcesBoxStripes;
    wxButton	*btnSourcesStripeColour;

    wxCheckBox	*chkPlaylistBorder;
    wxButton	*btnPlaylistBorderColour;
};
#endif


#ifndef OPTIONPLAYLISTPANEL_H
#define OPTIONPLAYLISTPANEL_H

#include "PrefPanel.h"

class OptionPlaylistPanel : public PrefPanel
{
public:
    OptionPlaylistPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Playlist" );
    }


protected:
    virtual void DoLoadPrefs();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();


private:
    wxCheckBox* chkPLColumnEnable[PlaylistColumn::NCOLUMNS];
    wxRadioBox*	rdbPLColumnStatic[PlaylistColumn::NCOLUMNS];
};
#endif

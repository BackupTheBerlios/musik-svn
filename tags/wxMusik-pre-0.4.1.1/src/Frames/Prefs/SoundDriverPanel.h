#ifndef SOUNDDRIVERPANEL_H 
#define SOUNDDRIVERPANEL_H

#include "PrefPanel.h"

class SoundDriverPanel : public PrefPanel
{
public:
    SoundDriverPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Driver" );
    }
    DECLARE_EVENT_TABLE()
protected:
    void DoLoadPrefs();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

    void OnOutputChanged	( wxCommandEvent& WXUNUSED(event) )	
    {	FindDevices();			}

    void FindDevices();
private:
    //-----------------------//
    //--- sound -> driver ---//
    //-----------------------//
    wxComboBox	*cmbOutputDrv;
    wxComboBox	*cmbSndDevice;
    wxComboBox	*cmbPlayRate;
    wxTextCtrl	*tcBufferLength;
    wxSpinCtrl * scSndMaxChan;

};
#endif
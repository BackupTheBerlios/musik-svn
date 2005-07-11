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
    virtual void AfterDataTransferredFromWindow();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();

    void OnClickColour		( wxCommandEvent& event );
    DECLARE_EVENT_TABLE()
private:
    struct DbChacheSizeSink : public IOnValueChangeByValidator
    {
        virtual void OnValueChange(wxGenericValidator *pV,wxWindowBase *pW);  
    };

    struct NeedUpdateSink: public IOnValueChangeByValidator
    {
        NeedUpdateSink()
        {
            m_bPlaylistUpdate = false;
            m_bActivityUpdate = false;
            m_bSourcesUpdate = false;
       }
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bPlaylistUpdate = true;
            m_bActivityUpdate = true;
            m_bSourcesUpdate = true;
        }
        static bool m_bPlaylistUpdate;
        static bool m_bActivityUpdate;
        static bool m_bSourcesUpdate;

    };

    struct PlaylistNeedUpdateSink : public NeedUpdateSink
    {
       virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bPlaylistUpdate = true;
        }

    };
    struct ActivityNeedUpdateSink : public NeedUpdateSink
    {
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bActivityUpdate = true;
        }

    };
    struct SourcesNeedUpdateSink : public NeedUpdateSink
    {
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bSourcesUpdate = true;
        }

    };

    struct PlaylistSourcesNeedUpdateSink : public NeedUpdateSink
    {
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bSourcesUpdate = true;
            m_bPlaylistUpdate = true;
        }

    };
    struct PlaylistActivityNeedUpdateSink : public NeedUpdateSink
    {
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bPlaylistUpdate = true;
            m_bActivityUpdate = true;
        }

    };
    struct SourcesActivityNeedUpdateSink : public NeedUpdateSink
    {
        virtual void OnValueChange(wxGenericValidator *,wxWindowBase *)
        {
            m_bSourcesUpdate = true;
            m_bActivityUpdate = true;
        }

    };

    PlaylistNeedUpdateSink snkPlaylistNeedUpdate;
    ActivityNeedUpdateSink snkActivityNeedUpdate;
    SourcesNeedUpdateSink snkSourcesNeedUpdate;
    PlaylistActivityNeedUpdateSink snkPlaylistActivityNeedUpdate;
    PlaylistSourcesNeedUpdateSink snkPlaylistSourcesNeedUpdate;

    wxButton	*btnPlaylistStripeColour;
    wxButton	*btnActivityStripeColour;
    wxButton	*btnSourcesStripeColour;
    wxButton	*btnPlaylistBorderColour;

    DbChacheSizeSink snkDBCacheSize;
};
#endif


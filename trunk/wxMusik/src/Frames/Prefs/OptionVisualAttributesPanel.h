#ifndef OPTIONVISUALATTRIBUTESPANEL_H
#define OPTIONVISUALATTRIBUTESPANEL_H
#include <wx/clrpicker.h>
#include "PrefPanel.h"

class OptionVisualAttributesPanel : public PrefPanel
{
public:
    OptionVisualAttributesPanel(wxWindow *parent);
    virtual wxString Name()
    {
        return _( "Visual Attributes" );
    }

protected:
    virtual void DoLoadPrefs();
    virtual void AfterDataTransferredFromWindow();
    virtual bool DoSavePrefs();
    wxSizer *CreateControls();
	void OnColourChange_PLStripeColour(wxColourPickerEvent& event);
	void OnColourChange_ActivityStripeColour(wxColourPickerEvent& event);
	void OnColourChange_SourcesStripeColour(wxColourPickerEvent& event);
	void OnColourChange_PLBorderStripeColour(wxColourPickerEvent& event);
	void OnTransparentWindowAlpha(wxScrollEvent& event);
    DECLARE_EVENT_TABLE()

	wxTopLevelWindow * GetTopLevelParent();
private:

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

};
#endif


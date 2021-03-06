#include "myprec.h"
// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/image.h"
#include "wx/wxhtml.h"
#include "wx/statline.h"

#include "MusikAboutDlg.h"
#include "MusikApp.h"
#include "wx/html/m_templ.h"

#include "MusikDefines.h"
#include "MusikUtils.h"
#include <sqlite3.h>

#include <taglib.h>
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT
#include <FLAC/format.h>
#endif
#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT
#include "neaacdec.h"
#endif

#ifndef MUSIKENGINE_NO_APE_SUPPORT
#undef VERSION
#undef PACKAGE
#undef PACKAGE_NAME 
#undef PACKAGE_STRING 
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "Shared/All.h" // monkeys audio
const char  MACLIB_VERSION[] = VERSION;
#undef VERSION
#undef PACKAGE
#undef PACKAGE_NAME 
#undef PACKAGE_STRING 
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#ifdef _WIN32
#else
#define BUILD_CROSS_PLATFORM
#endif
#endif //#ifndef MUSIKENGINE_NO_APE


#ifndef MUSIKENGINE_NO_MPC_SUPPORT	
#undef VERSION
#undef PACKAGE
#undef PACKAGE_NAME 
#undef PACKAGE_STRING 
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#include <3rd-Party/libmpcdec/include/config.h>
const char  MPCDEC_VERSION[] = VERSION;
#undef VERSION
#undef PACKAGE
#undef PACKAGE_NAME 
#undef PACKAGE_STRING 
#undef PACKAGE_STRING 
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#endif
DECLARE_APP( MusikApp )

TAG_HANDLER_BEGIN(MUSIK_TAG, "MUSIK")

TAG_HANDLER_PROC(tag)
{

	wxString sTextToInsert;
	wxString sParamValue;
	if (tag.HasParam(wxT("Version")))
	{
		sParamValue = tag.GetParam(wxT("Version"));
		if(	sParamValue == wxT("wxMusik"))
		{
			sTextToInsert = MUSIK_VERSION_STR;
		}
		if(	sParamValue == wxT("wxMusikEx"))
		{
#ifdef  wxUSE_UNICODE
			sTextToInsert = wxT("UNICODE ");
#endif
			sTextToInsert += MUSIK_VERSION_DEBUG MUSIK_VERSION_ADDENDUM;
		}
		else if(	sParamValue == wxT("sqlite"))
		{
			
			sTextToInsert = ConvA2W(sqlite3_version);
		}
		else if(sParamValue == wxT("wxWidgets"))
		{
#define WX_VERSION_STR		MUSIK_STRINGIZE_T(wxMAJOR_VERSION) wxT(".") \
								MUSIK_STRINGIZE_T(wxMINOR_VERSION) wxT(".") \
								MUSIK_STRINGIZE_T(wxRELEASE_NUMBER) 
			sTextToInsert =WX_VERSION_STR;

		}
		else if(sParamValue == wxT("engine"))
		{
			sTextToInsert = ConvA2W(wxGetApp().Player.SndEngine().Version());

		}
		else if(sParamValue == wxT("taglib"))
		{
			sTextToInsert	<< TAGLIB_MAJOR_VERSION << wxT(".") 
							<< TAGLIB_MINOR_VERSION << wxT(".") 
							<< TAGLIB_PATCH_VERSION; 

		}
		else if(sParamValue == wxT("OggVorbis"))
		{
			sTextToInsert = wxT("uses fmod");
		}		
		else if(sParamValue == wxT("libFlac"))
		{
#ifndef MUSIKENGINE_NO_FLAC_SUPPORT		
			sTextToInsert = ConvA2W(FLAC__VERSION_STRING);
#else
			sTextToInsert = _("Not supported");
#endif			
		}
		else if(sParamValue == wxT("libfaad2"))
		{
#ifndef MUSIKENGINE_NO_FAAD2_SUPPORT		
			sTextToInsert = ConvA2W(FAAD2_VERSION);
#else
			sTextToInsert = _("Not supported");
#endif			
		}
		else if(sParamValue == wxT("MACSDK"))
		{
#ifndef MUSIKENGINE_NO_APE_SUPPORT		
			sTextToInsert = ConvA2W(MACLIB_VERSION);
#else
			sTextToInsert = _("Not supported");
#endif			
		}
		else if(sParamValue == wxT("libmpcdec"))
		{
#ifndef MUSIKENGINE_NO_MPC_SUPPORT		
			sTextToInsert = ConvA2W(MPCDEC_VERSION);
#else
			sTextToInsert = _("Not supported");
#endif			
		}
		else if(sParamValue == wxT("Shibatch"))
		{
			sTextToInsert = wxT("0.3");
		}
		m_WParser->GetContainer()->InsertCell(new wxHtmlWordCell(sTextToInsert, *(m_WParser->GetDC())));
		return FALSE;
	}


	return FALSE;
}

TAG_HANDLER_END(MUSIK_TAG)



TAGS_MODULE_BEGIN(MUSIK_TAG)

TAGS_MODULE_ADD(MUSIK_TAG)

TAGS_MODULE_END(MUSIK_TAG)

class MyHtmlWindow : public wxHtmlWindow {
public:
    MyHtmlWindow(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxHW_DEFAULT_STYLE)
        : wxHtmlWindow(parent, id, pos, size, style) { }
        virtual void OnLinkClicked(const wxHtmlLinkInfo& link);
};

void MyHtmlWindow::OnLinkClicked(const wxHtmlLinkInfo& link) {
    wxString url = link.GetHref();
    if ( url.StartsWith(wxT("http:")) || url.StartsWith(wxT("mailto:")) ) 
    {
        // pass http/mailto URL to user's preferred browser/emailer
#ifdef __WXMAC__
        // wxLaunchDefaultBrowser doesn't work on Mac with IE
        // but it's easier just to use the Mac OS X open command
        if ( wxExecute(wxT("open ") + url, wxEXEC_ASYNC) == -1 )
            wxLogWarning(wxT("Could not open URL!"));
#else
        if ( !wxLaunchDefaultBrowser(url) )
            wxLogWarning(wxT("Could not launch browser!"));
#endif
    } 
    else 
    {
        LoadPage(url);
    }
}

CMusikAboutDlg::CMusikAboutDlg(wxWindow *pParent)
:wxDialog(pParent, wxID_ANY, wxString(_("About")),wxDefaultPosition,wxDefaultSize)
{
	
	

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

	wxHtmlWindow *html = new MyHtmlWindow(this,wxID_ANY,wxDefaultPosition,wxSize(120, 10)/* wxDefaultSize*/);
	html -> SetBorders(0);
	html -> LoadPage(MusikGetStaticDataPath() + wxT("about.html"));
	html -> SetSize(html -> GetInternalRepresentation() -> GetWidth()+wxSystemSettings::GetMetric(wxSYS_VSCROLL_X), 
		400);//html -> GetInternalRepresentation() -> GetHeight());

	topsizer -> Add(html, 1, wxBOTTOM, 10);

	topsizer -> Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	wxButton *bu1 = new wxButton(this, wxID_OK, _("OK"));
	bu1 -> SetDefault();
	topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);
	SetAutoLayout(true);
	SetSizer(topsizer);
	topsizer->Fit(this);
	Centre();
}

CMusikAboutDlg::~CMusikAboutDlg(void)
{
}

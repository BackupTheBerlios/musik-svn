#ifndef PREFUTIL_H
#define PREFUTIL_H
#include "MusikApp.h"
#include "MusikUtils.h"
#include <wx/spinctrl.h>
#include <wx/checklst.h>
#include <wx/valgen.h>

#define PREF_CREATE_CHECKBOX(prefname,text) \
	wxCheckBox *chk##prefname	=	new wxCheckBox_NoFlicker( this, -1,text,wxDefaultPosition,wxDefaultSize,0 \
	,wxGenericValidator(&wxGetApp().Prefs.b##prefname));

#define PREF_CREATE_CHECKBOX_EX(prefname,text,psnk) \
    wxCheckBox *chk##prefname	=	new wxCheckBox_NoFlicker( this, -1,text,wxDefaultPosition,wxDefaultSize,0 \
    ,wxBoolValidator(&wxGetApp().Prefs.b##prefname,psnk));

#define PREF_CREATE_SPINCTRL(prefname,minval,maxval,curval) \
	wxSpinCtrl *sc##prefname	= new wxSpinCtrl( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,minval,maxval,curval);\
{   	wxIntValidator v(&wxGetApp().Prefs.n##prefname);		\
	sc##prefname->SetValidator(v);								}
#define PREF_CREATE_SPINCTRL_EX(prefname,minval,maxval,psnk) \
    wxSpinCtrl *sc##prefname	= new wxSpinCtrl( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,minval,maxval);\
{   	wxIntValidator v(&wxGetApp().Prefs.n##prefname,psnk);		\
    sc##prefname->SetValidator(v);								}

#define PREF_CREATE_SPINCTRL2(prefname,minval,maxval,curval) \
	sc##prefname	= new wxSpinCtrl( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,minval,maxval,curval);\
{   	wxIntValidator v(&wxGetApp().Prefs.n##prefname);		\
	sc##prefname->SetValidator(v);								}
#define PREF_CREATE_TEXTCTRL(prefname,valmode)	 \
	wxTextCtrl *tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,0 \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));
#define PREF_CREATE_TEXTCTRL2(prefname,valmode)	 \
	tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,0 \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));

#define PREF_CREATE_MULTILINETEXTCTRL(prefname,height,valmode)	 \
	wxTextCtrl *tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxSize(-1,height),wxTE_MULTILINE \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));
#define PREF_CREATE_MULTILINETEXTCTRL2(prefname,height,valmode)	  \
	tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxSize(-1,height),wxTE_MULTILINE \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));

#define PREF_STATICTEXT(text)	\
	new wxStaticText_NoFlicker( this, -1, text)

#endif

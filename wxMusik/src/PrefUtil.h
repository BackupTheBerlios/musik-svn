#ifndef PREFUTIL_H
#define PREFUTIL_H

#include "MusikUtils.h"

#define PREF_CREATE_CHECKBOX(prefname,text) \
	wxCheckBox *chk##prefname	=	new wxCheckBox_NoFlicker( this, -1,text,wxDefaultPosition,wxDefaultSize,0 \
	,wxGenericValidator(&wxGetApp().Prefs.b##prefname));

#define PREF_CREATE_SPINCTRL(prefname,minval,maxval,curval) \
	wxSpinCtrl *sc##prefname	= new wxSpinCtrl( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,minval,maxval,curval);\
{   	wxGenericIntValidator v(&wxGetApp().Prefs.n##prefname);		\
	sc##prefname->SetValidator(v);								}
#define PREF_CREATE_SPINCTRL2(prefname,minval,maxval,curval) \
	sc2##prefname	= new wxSpinCtrl( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,minval,maxval,curval);\
{   	wxGenericIntValidator v(&wxGetApp().Prefs.n##prefname);		\
	sc2##prefname->SetValidator(v);								}
#define PREF_CREATE_TEXTCTRL(prefname,valmode)	 \
	wxTextCtrl *tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,0 \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));
#define PREF_CREATE_TEXTCTRL2(prefname,valmode)	 \
	tc2##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxDefaultSize,0 \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));

#define PREF_CREATE_MULTILINETEXTCTRL(prefname,height,valmode)	 \
	wxTextCtrl *tc##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxSize(-1,height),wxTE_MULTILINE \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));
#define PREF_CREATE_MULTILINETEXTCTRL2(prefname,height,valmode)	  \
	tc2##prefname	=	new wxTextCtrl_NoFlicker( this, -1,wxEmptyString,wxDefaultPosition,wxSize(-1,height),wxTE_MULTILINE \
	,wxTextValidator(valmode,&wxGetApp().Prefs.s##prefname));

#define PREF_STATICTEXT(text)	\
	new wxStaticText_NoFlicker( this, -1, text)

#endif

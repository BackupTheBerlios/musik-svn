/////////////////////////////////////////////////////////////////////////////
// Name:        MusikAutoTaggerFrame.h
// Purpose:     Selecting auto tag mask
// Author:      Gunnar Roth
// Modified by: 
// Created:     12/28/03 16:11:36
// RCS-ID:      
// Copyright:  Gunnar Roth 2004 
// Licence:  See the file "license.txt" for information on usage and redistribution  
/////////////////////////////////////////////////////////////////////////////

#ifndef _MUSIKAUTOTAGGERFRAME_H_
#define _MUSIKAUTOTAGGERFRAME_H_

#ifdef __GNUG__
#pragma interface "MusikAutoTaggerFrame.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DLGMUSIKAUTOTAGGER 10000
#define ID_COMBOBOX 10001
#define ID_BN_ADDMASK 10002
#define ID_BN_REMOVEMASK 10003
#define ID_CHECKBOX 10004
////@end control identifiers

/*!
 * CMusikAutoTaggerFrame class declaration
 */

class CMusikAutoTaggerFrame: public wxDialog
{    
    DECLARE_CLASS( CMusikAutoTaggerFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMusikAutoTaggerFrame( );
    CMusikAutoTaggerFrame( wxWindow* parent, wxWindowID id = -1, const wxString& caption = wxString(MUSIKAPPNAME) + _(" Autotagger"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxRESIZE_BORDER|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = -1, const wxString& caption = wxString(MUSIKAPPNAME) + _(" Autotagger"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxRESIZE_BORDER|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMusikAutoTaggerFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BN_ADDMASK
    void OnBnAddMask( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BN_REMOVEMASK
    void OnBnRemoveMask( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOk( wxCommandEvent& event );

////@end CMusikAutoTaggerFrame event handler declarations

////@begin CMusikAutoTaggerFrame member function declarations

    wxString GetMask() const { return m_sMask ; }
    void SetMask(wxString value) { m_sMask = value ; }

    bool GetConvertUnderscoresToSpaces() const { return m_bConvertUnderscoresToSpaces ; }
    void SetConvertUnderscoresToSpaces(bool value) { m_bConvertUnderscoresToSpaces = value ; }

////@end CMusikAutoTaggerFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMusikAutoTaggerFrame member variables
    wxComboBox* m_CBTagMask;
    wxString m_sMask;
    bool m_bConvertUnderscoresToSpaces;
////@end CMusikAutoTaggerFrame member variables
};

#endif
    // _MUSIKAUTOTAGGERFRAME_H_

/*
 *  MusikUtils.h
 *
 *  Global functions that don't really belong 
 *  anywhere but here.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIKUTILS_H
#define MUSIKUTILS_H

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include <wx/regex.h>
#include <wx/listctrl.h>
#include <wx/textdlg.h>
#include <wx/mimetype.h>
#include "MusikDefines.h"

class CSongPath
{
public:
	CSongPath()
	{
	}
 
   explicit CSongPath(const wxString &s)
	   :m_sPath(s)
   {
   }
   ~CSongPath()
   {

   }

   const CSongPath & operator=(const wxString &sz)
   {
	   m_sPath = sz;
	   return *this;
   }
   const CSongPath & operator=(const wxFileName & fn)
   {
	   m_sPath = fn.GetFullPath(wxPATH_NATIVE);
	   return *this;
   }
   const CSongPath & operator=(const CSongPath & rhs)
   {
	   if(this != &rhs)
	   {
		   m_sPath = rhs.m_sPath;
	   }
	   return *this;
   }

   bool operator==(const CSongPath & rhs) const
   {
	   	   return m_sPath == rhs.m_sPath;
   }
   bool operator==(const wxFileName & rhs) const
   {
	   return m_sPath == rhs.GetFullPath(wxPATH_NATIVE);
   }
   bool operator !=(const wxFileName & rhs) const
   {
	   return !(*this == rhs);
   }

   operator wxFileName () const
   {
	   return wxFileName(m_sPath,wxPATH_NATIVE);
   }
  const wxString & GetFullPath() const
   {
	   return m_sPath;
   }
   bool FileExists() const
   {
	   if(!IsUrl())
		   return wxFileExists(m_sPath);
	   return true;
   }
   bool IsUrl()	const
   {
	   return m_sPath.StartsWith(wxT("http://"));
   }
   bool IsFile() const
   {
		return !IsUrl();
   }
   wxString GetPath(int flags = wxPATH_GET_VOLUME,
	   wxPathFormat format = wxPATH_NATIVE) const
   {
	   wxFileName fn(m_sPath);
	   return fn.GetPath(   flags, format);
   }
   wxString GetExt() const
   {
		size_t pos = m_sPath.rfind('.');
		if(pos != wxStringBase::npos )
		{
			return m_sPath.Right(m_sPath.Len() - pos - 1);
		}
		return wxString();
   }
private:
	wxString m_sPath;
};

class CSongMetaData
{
public:
	class StringData
	{
	public:
		StringData()
		{
			m_szData = NULL;
			m_bOwner = false;
		}
		StringData(const char* sz , size_t len = (size_t) -1)
		{
			Assign(sz,len);
		}

		void Assign(const char* sz , size_t len = (size_t) -1)
		{
			m_szData = _StrDup(sz,len);
			m_bOwner = true;
		}
		off_t Find(int ch)   const
		{
			if(m_szData)
			{
				char *pos = strchr(m_szData,ch);
				return pos - m_szData;
			}
			return -1;
		}
		const char * Substr(off_t start)
		{
			return IsEmpty() ? "" : m_szData + start;
		}
		void Attach(const char* sz ,bool bOwner = true)
		{
			Empty();
			m_bOwner = bOwner;
			m_szData =  sz ;
		}
		StringData(const StringData & rhs)
		{
			m_bOwner = true;
			m_szData =  _StrDup(rhs.m_szData);
		}
		const StringData & operator=(const StringData & rhs)
		{
			if(this != &rhs)
			{
				Empty();
				m_bOwner = true;
				m_szData =  _StrDup(rhs.m_szData);
			}
			return *this;
		}
		const StringData & operator=(const char * sz)
		{
			Empty();
			m_bOwner = true;
			m_szData =  _StrDup(sz);
			return *this;
		}

		operator const char *()	const 
		{
			return IsEmpty() ? "" : m_szData;
		}
		bool IsEmpty()	const 
		{
			return (m_szData == NULL) || (Length() == 0);
		}
		void Empty()
		{
			if(m_szData && m_bOwner)
				delete [] (m_szData);
			m_szData = NULL;
		}
		const char * Detach()
		{
			const char * tmp = m_szData;
			m_szData = NULL;
			return tmp;
		}
		size_t Length()	const
		{
			return (m_szData == NULL) ? 0 : strlen(m_szData);
		}
		~StringData()
		{
			Empty();
		}
	protected:
		const char * _StrDup(const char * sz, size_t len=(size_t) -1)
		{
			if(!sz)
				return NULL;
			len = (len == (size_t) -1) ? strlen(sz) : len;
			char *buf = new char[len + 1];
			if(buf)
			{
				strncpy(buf,sz,len);
				buf[len] = 0;
			}
			return buf;
		}
	private:
		const char *m_szData;
		bool m_bOwner;
	};
public:
	CSongMetaData()
	{
		eFormat = MUSIK_FORMAT_INVALID;
		bVBR = false;
		nDuration_ms = nBitrate  = nFilesize   =	nTracknum = 0;
	}

	CSongPath			Filename;
	StringData			Title;
	StringData			Artist;
	StringData			Album;
	StringData			Genre;
	StringData			Year;
	StringData			Notes;
	int				    nTracknum;
	EMUSIK_FORMAT_TYPE  eFormat;
	int					nDuration_ms;
	bool				bVBR;
	int					nBitrate;
	int					nFilesize;

};


class CMusikSong
{
public:
	CMusikSong();

public:
	int			songid;
	CSongMetaData MetaData;
	double		LastPlayed;
	int			Rating;
	int			TimesPlayed;
	double		TimeAdded;
	unsigned int			Check1:1;		//--- for tag dlg stuff, checks to see if it needs to be written to file / db ---//
	unsigned int			bChosenByUser:1;
	unsigned int			bForcePlay:1;


};

#define MUSIK_LIB_ALL_SONGCOLUMNS	wxT(" songs.songid,")	  \
									wxT("songs.filename,")	  \
									wxT("songs.title,")	  \
									wxT("songs.tracknum,")	  \
									wxT("songs.artist,")	  \
									wxT("songs.album,")	  \
									wxT("songs.genre,")	  \
									wxT("songs.duration,")	  \
									wxT("songs.format,")	  \
									wxT("songs.vbr,")		  \
									wxT("songs.year,")		  \
									wxT("songs.rating,")	  \
									wxT("songs.bitrate,")	  \
									wxT("songs.lastplayed,") \
									wxT("songs.notes,")	  \
									wxT("songs.timesplayed,")\
									wxT("songs.timeadded,")  \
									wxT("songs.filesize ")

WX_DECLARE_OBJARRAY( CMusikSong, CMusikSongArray );


//------------------------------------------------------------------//
//--- misc utility functions, see implementatio		n for description ---//
//------------------------------------------------------------------//
// wxString		GetGenre			(  const wxString & sGenre );
int				GetGenreID			(  const CSongMetaData::StringData & sGenre  );

void			DelimitStr			( wxString sStr, wxString sDel, wxArrayString &aReturn, bool bRemoveDelimiter = true );

void			GetPlaylistDir		( wxArrayString & aFiles );
wxString		MStoStr				( int timems );
wxArrayString	FileToStringArray	(  const wxString &  sFilename );
int				MusikRound			( float x );
int				FindStrInArray		( wxArrayString* array, wxString pattern );
wxString		MoveArtistPrefixToEnd( const wxString & str );
const wxChar*	BeginsWithPreposition( const wxString & str );
wxString		SanitizedString		( const wxString & str );
void			wxListCtrlSelAll	( wxListCtrl* listctrl );
void			wxListCtrlSelNone	( wxListCtrl* listctrl );
wxString		ColourToString		( const wxColour& color );
wxColour		StringToColour		( const wxString& string );


void			SortArrayByLength	( wxArrayString* pArray );


wxString		GetFramePlacement	( wxFrame* frame );
bool			SetFramePlacement	( wxFrame* frame, wxString place );

void			SeedRandom			( unsigned long s );
long			GetRandomNumber		();
const int RandomMax = 0x7fffffffL;
//-------------------------//
//--- Unicode functions ---//
//-------------------------//
inline wxString ConvA2W( const char *pChar )
{
	wxString s(wxConvISO8859_1.cMB2WX( pChar ) );
	return s;
}
inline const wxCharBuffer ConvW2A( const wxString &s )
{
	return wxConvISO8859_1.cWX2MB( s );
}
inline const wxCharBuffer ConvFn2A( const wxString &s )
{
	return wxConvFile.cWX2MB( s );
}


inline const wxCharBuffer ConvToUTF8( const wxString &s )
{
#if wxUSE_UNICODE
	return wxConvUTF8.cWC2MB(s);
#else
	return wxConvUTF8.cWC2WX(wxConvCurrent->cMB2WC(s));
#endif
}
inline const wxCharBuffer ConvFromISO8859_1ToUTF8( const char *s )
{
	return wxConvUTF8.cWC2MB(wxConvISO8859_1.cMB2WC(s));
}


#if wxUSE_UNICODE
inline const wxWCharBuffer ConvFromUTF8( const char *s )
{
	wxWCharBuffer buf(wxConvUTF8.cMB2WC(s));
	if(buf.data())
		return buf;
	return wxWCharBuffer(L"");
}
#else
inline const wxCharBuffer ConvFromUTF8( const char *s )
{
	wxWCharBuffer buf(wxConvUTF8.cMB2WC(s));
	if(buf.data())
		return wxConvCurrent->cWC2WX(buf);
	return wxCharBuffer("");
}
#endif

inline const wxCharBuffer ConvFromUTF8ToISO8859_1( const char *s )
{
#if wxUSE_UNICODE
	return wxConvISO8859_1.cWX2MB(	ConvFromUTF8( s )  );
#else
	return wxConvISO8859_1.cWC2WX(wxConvUTF8.cMB2WC(s));
#endif

}
inline const wxCharBuffer ConvQueryToMB( const wxString &s )
{
	return ConvToUTF8(s);
}


inline int wxStringToInt( const wxString &str )
{
	long lRet;
	str.ToLong( &lRet );
	return (int)lRet;
}

inline int StringToInt( const char* pChar )
{
	return pChar ? atoi( pChar ): 0;
}

inline wxString IntTowxString( int n )
{
	wxString ret;
	ret << n;
	return ret;
}
inline double StringToDouble( wxString str )
{
	double ret;
	str.ToDouble( &ret );
	return ret;
}
inline void InternalErrorMessageBox( const wxString &sText)
{
	wxString sMessage = wxString::Format(wxT("An internal error has occured.\n%s,\n\nPlease contact the ") MUSIKAPPNAME wxT(" development team with this error."),(const wxChar *)sText);
	wxMessageBox( sMessage,MUSIKAPPNAME_VERSION, wxOK|wxICON_ERROR );

}
inline wxString MusikGetStaticDataPath()
{
#ifdef __WXMSW__
	return wxT("data/");		
#else
	wxString sDataPath(wxT("/usr/local/share/") MUSIKAPPNAME wxT("/data/"));
	if(wxDirExists(sDataPath))
		return sDataPath;
	sDataPath = wxT("/usr/share/") MUSIKAPPNAME wxT("/data/");	
	if(wxDirExists(sDataPath))
		return sDataPath;
	sDataPath = wxT("~/") MUSIKAPPNAME wxT("/data/");	
	if(wxDirExists(sDataPath))
		return sDataPath;
	return wxT("data/");

#endif	
}
double CharStringToDouble(const char *z);
void DoubleToCharString(double r, char *z);
wxString GetForbiddenChars(wxPathFormat format = wxPATH_NATIVE);

void  ReplaceChars(wxString &s,const wxString &chars,wxChar replaceby = wxT('_'));

wxLongLong GetTotalFilesize(const CMusikSongArray &songs);

wxString SecToStr	( int nSec );

bool GetFileTypeAssociationInfo(const wxString &sExt,wxString *psDescription,bool * pbAssociatedWithApp);
inline bool FileTypeIsAssociated(const wxString &sExt)
{
	bool bAssociated = false;
	return GetFileTypeAssociationInfo(sExt,NULL,&bAssociated) && bAssociated;
}
bool FileTypeIsAssociated(const wxFileType &ft);
void AssociateWithFileType(const wxString &sExt,const wxString &sDescription);
void UnassociateWithFileType(const wxString &sExt);
class CNiceFilesize
{
public:
	CNiceFilesize();
	~CNiceFilesize();

public:
	long	m_Bytes;
	long	m_Kilobytes;
	long	m_Megabytes;
	long	m_Gigabytes;
	long	m_Terabytes;

	void	AddB( long b );
	void	AddK( long k );
	void	AddM( long m );
	void	AddG( long g );
	void	AddT( long t );

	wxString	GetFormatted();
};

class CMusikSong;

class CMusikTagger
{

	wxArrayInt 	m_PlaceHolderArray;
	wxRegEx 	m_reMask;
	bool		m_bConvertUnderscoresToSpaces;
public:
	CMusikTagger(const wxString &sMask, bool bConvertUnderscoresToSpaces);
	bool Retag(CMusikSong * Song) const;
};

// styles
#define	 MUSIK_LW_ClearContentOnClose 0x00000001
#define	 MUSIK_LW_DoPass			  0x00000002
#define	 MUSIK_LW_ShowInitial		  0x00000004
#define	 MUSIK_LW_ShowOnLog			  0x00000008


class  MusikLogWindow : public wxEvtHandler,public wxLogWindow
{
public:
	MusikLogWindow(wxFrame *pParent,         // the parent frame (can be NULL)
		const wxChar *szTitle,    // the title of the frame
		long style);  //see above  (MUSIK_LW_XXX)

	virtual bool OnFrameClose(wxFrame *frame);
	void OnNewMsg(wxCommandEvent &);
	DECLARE_EVENT_TABLE()
protected:
	virtual void DoLogString(const wxChar *szString, time_t t);

private:
	long	m_Style;	
	DECLARE_NO_COPY_CLASS(MusikLogWindow)
};

class  wxMultiLineTextEntryDialog : public wxDialog
{
public:
	wxMultiLineTextEntryDialog(wxWindow *parent,
		const wxString& message,
		const wxString& caption = wxGetTextFromUserPromptStr,
		const wxString& value = wxEmptyString,
		long style = wxTextEntryDialogStyle,
		const wxPoint& pos = wxDefaultPosition,const wxSize & size = wxDefaultSize);

	void SetValue(const wxString& val);
	wxString GetValue() const { return m_value; }

#if wxUSE_VALIDATORS
	void SetTextValidator( wxTextValidator& validator );
	void SetTextValidator( long style = wxFILTER_NONE );
	wxTextValidator* GetTextValidator() { return (wxTextValidator*)m_textctrl->GetValidator(); }
#endif
	// wxUSE_VALIDATORS

	// implementation only
	void OnOK(wxCommandEvent& event);

protected:
	wxTextCtrl *m_textctrl;
	wxString    m_value;
	long        m_dialogStyle;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(wxMultiLineTextEntryDialog)
	DECLARE_NO_COPY_CLASS(wxMultiLineTextEntryDialog)
};

class wxStaticText_NoFlicker : public wxStaticText
{
public:
	wxStaticText_NoFlicker(wxWindow* parent,
		wxWindowID id,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name= wxStaticTextNameStr )
			:wxStaticText(parent,id,label,pos,size,style,name){}
	void OnEraseBackground(wxEraseEvent& ) {}
	DECLARE_EVENT_TABLE()
};
class wxCheckBox_NoFlicker : public wxCheckBox
{
public:
	wxCheckBox_NoFlicker( wxWindow*          pParent
		,wxWindowID         vId
		,const wxString&    rsLabel
		,const wxPoint&     rPos = wxDefaultPosition
		,const wxSize&      rSize = wxDefaultSize
		,long lStyle = 0
		,const wxValidator& rValidator = wxDefaultValidator
		,const wxString&    rsName = wxCheckBoxNameStr
		):
	wxCheckBox(pParent,vId,rsLabel,rPos,rSize,lStyle,rValidator,rsName){}
	void OnEraseBackground(wxEraseEvent& ) {}
	DECLARE_EVENT_TABLE()
};

class wxChoice_NoFlicker : public wxChoice
{
public:
	wxChoice_NoFlicker(wxWindow *parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		int n = 0, const wxString choices[] = (const wxString *) NULL,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxChoiceNameStr)
			:wxChoice(parent,id,pos,size,n,choices,style,validator,name){}
	wxChoice_NoFlicker(wxWindow *parent, wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		const wxArrayString& choices,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxChoiceNameStr)
			:wxChoice(parent,id,pos,size,choices,style,validator,name){}
	void OnEraseBackground(wxEraseEvent& ) {}
	DECLARE_EVENT_TABLE()
};
class wxButton_NoFlicker : public wxButton
{
public:
	wxButton_NoFlicker(wxWindow *parent,
		wxWindowID id,
		const wxString& label,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxButtonNameStr)
			:wxButton(parent,id,label,pos,size,style,validator,name){}
	void OnEraseBackground(wxEraseEvent& ) {}
	DECLARE_EVENT_TABLE()
};

class wxTextCtrl_NoFlicker : public wxTextCtrl
{
public:
	wxTextCtrl_NoFlicker(wxWindow *parent,
		wxWindowID id,
		const wxString &value = wxEmptyString,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString &name = wxTextCtrlNameStr)
			:wxTextCtrl(parent,id,value,pos,size,style,validator,name){}
		void OnEraseBackground(wxEraseEvent& event) { event.Skip();}
		DECLARE_EVENT_TABLE()
};

#include <wx/valgen.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>

class WXDLLEXPORT wxGenericIntValidator: public wxGenericValidator
{
	DECLARE_CLASS(wxGenericIntValidator)
public:
	wxGenericIntValidator(int* val)
		:wxGenericValidator(val)
	{
	}

	virtual wxObject *Clone() const { return new wxGenericIntValidator(*this); }

	// Called when the value in the window must be validated.
	// This function can pop up an error message.
	virtual bool Validate(wxWindow *  WXUNUSED(parent)) 
	{
#if wxUSE_SPINCTRL && !defined(__WXMOTIF__)
		if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl)) )
		{
			wxSpinCtrl* pControl = (wxSpinCtrl*) m_validatorWindow;
			return pControl && IsValid(pControl->GetValue(),pControl->GetMin(),pControl->GetMax());
		} else
#endif
#if wxUSE_SPINBTN
		if (m_validatorWindow->IsKindOf(CLASSINFO(wxSpinButton)) )
		{
			wxSpinButton* pControl = (wxSpinButton*) m_validatorWindow;
			return pControl && IsValid(pControl->GetValue(), pControl->GetMin(),pControl->GetMax());
		} else
#endif
		{
		}
		return true; 
	}
protected:
	bool IsValid(int val,int min,int max) const
	{
		if (val)
		{
			if(val >= min && val <= max)
				return true;
		}
		return false;
	}
};

#endif

#ifdef __WXMSW__
#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD GetDllVersion(LPCTSTR lpszDllName);
#endif


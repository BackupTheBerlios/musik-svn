#ifndef DNDHELPER_H
#define DNDHELPER_H

#ifdef __WXMAC__// on mac the custom string may have only up to 4 chars
#define MUSIK_SOURCES_DATAID wxT("wMSd")
#define MUSIK_SONGLIST_DATAID wxT("wMSl")
#else
#define MUSIK_SOURCES_DATAID wxT("wxMusik/SourcesData")
#define MUSIK_SONGLIST_DATAID wxT("wxMusik/SonglistData")
#endif
class CMusikInternalDataObject : public wxTextDataObject
{
public:
	CMusikInternalDataObject(const wxString & dfname ,const wxString& text = wxEmptyString )
		:wxTextDataObject(text)
	{
		SetFormat(wxDataFormat(dfname));
	}
protected:
private:
};

class CMusikSonglistDataObject : public CMusikInternalDataObject
{
public:
	CMusikSonglistDataObject(const wxString& text = wxEmptyString )
		:CMusikInternalDataObject(wxString(MUSIK_SONGLIST_DATAID), text )
	{
	}
};

class CMusikSourcesDataObject : public CMusikInternalDataObject
{
public:
	CMusikSourcesDataObject(const wxString& text = wxEmptyString )
		:CMusikInternalDataObject(wxString(MUSIK_SOURCES_DATAID), text )
	{
	}
};


#endif // DNDHELPER_H

#ifndef DNDHELPER_H
#define DNDHELPER_H

#define MUSIK_SOURCES_DATAID wxT("wxMusik/SourcesData")
#define MUSIK_SONGLIST_DATAID wxT("wxMusik/SonglistData")

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

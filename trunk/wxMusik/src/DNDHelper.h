#ifndef DNDHELPER_H
#define DNDHELPER_H

#ifdef __WXMAC__// on mac the custom string may have only up to 4 chars
#define MUSIK_SOURCES_DATAID wxT("wMSd")
#define MUSIK_SONGLIST_DATAID wxT("wMSl")
#else
#define MUSIK_SOURCES_DATAID wxT("wxMusik/SourcesData")
#define MUSIK_SONGLIST_DATAID wxT("wxMusik/SonglistData")
#endif
class CMusikInternalDataObject : public wxDataObjectSimple
{
public:
	CMusikInternalDataObject(const wxString & dfname ,const wxString& text = wxEmptyString )
        : wxDataObjectSimple(wxDataFormat(dfname)),
          m_text(text)
 	{
	}
    // virtual functions which you may override if you want to provide text on
    // demand only - otherwise, the trivial default versions will be used
    virtual size_t GetTextLength() const { return m_text.Len() + 1; }
    virtual wxString GetText() const { return m_text; }
    virtual void SetText(const wxString& text) { m_text = text; }


    size_t GetDataSize() const
    {
         return GetTextLength() * sizeof(wxChar);
    }

    bool GetDataHere(void *buf) const
    {
        wxStrcpy((wxChar *)buf, GetText().c_str());
        return true;
    }

    bool SetData(size_t WXUNUSED(len), const void *buf)
    {
        SetText(wxString((const wxChar *)buf));
        return true;
    }

    size_t GetDataSize(const wxDataFormat& format) const
            { return(wxDataObjectSimple::GetDataSize(format)); }
    bool GetDataHere(const wxDataFormat& format, void *pBuf) const
            { return(wxDataObjectSimple::GetDataHere(format, pBuf)); }
    bool SetData(const wxDataFormat& format, size_t nLen, const void* pBuf)
            { return(wxDataObjectSimple::SetData(format, nLen, pBuf)); }
                                    	
protected:
private:
    wxString m_text;

    DECLARE_NO_COPY_CLASS(CMusikInternalDataObject)

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

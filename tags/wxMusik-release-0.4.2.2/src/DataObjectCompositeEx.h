#ifndef DATAOBJECTCOMPOSITEEX_H
#define DATAOBJECTCOMPOSITEEX_H
class wxDataObjectCompositeEx : public wxDataObjectComposite
{
public:
	wxDataObjectCompositeEx()
	{
		m_dataObjectLast = NULL;
	}

	bool SetData(const wxDataFormat& format,
		size_t len,
		const void *buf)
	{
		m_dataObjectLast = GetObject(format);

		wxCHECK_MSG( m_dataObjectLast, FALSE,
			wxT("unsupported format in wxDataObjectCompositeEx"));

		return m_dataObjectLast->SetData(len, buf);
	}

	wxDataObjectSimple *GetActualDataObject()
	{
		return m_dataObjectLast;
	}
private:
	wxDataObjectSimple *m_dataObjectLast;
};
#endif // DATAOBJECTCOMPOSITEEX_H

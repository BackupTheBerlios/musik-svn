/*
 *  MusikDb.h
 *
 *  Abstrct Class which controls database manipulation.
 *
 *  
 *  Copyright (c) 2006 Gunnar Roth (gunnar.roth@gmx.de)
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_DB_H
#define MUSIK_DB_H


//--- wx ---//
#include "myprec.h"
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include <map>
//--- sql ---//
#include "sqlite.h"

#include "MusikUtils.h"
#include "Playlist.h"


class MusikDb : public wxEvtHandler
{
public:
    class QueryString
    {
    public:
        QueryString(const char * format,...)
        {
            va_list args;
            va_start(args, format);

            m_pStr = sqlite_vmprintf(format,args);
            va_end(args);

        }
        QueryString(const wxString & q)
        {
            m_pStr = sqlite_mprintf("%s",ConvToUTF8(q).data());
        }
        operator const char*() const
        {
            return m_pStr;   
        }
        ~QueryString()
        {
            if(m_pStr)
                sqlite_freemem(m_pStr);
        }
    private:
        char * m_pStr;
        QueryString(const QueryString&);
        QueryString &operator=(const QueryString &);
    };

    class DbError
    {
    public:
        DbError()
        {
            m_errmsg = NULL;
            m_err = SQLITE_OK;
        }
        virtual ~DbError()
        {
            if(Code())
            {
                wxLogError(wxT("%s"),ConvA2W(Text()).c_str());
            }
            Clear();
        }
        const char *Text() const
        {
            return m_errmsg ?  m_errmsg : (m_err == SQLITE_OK ? "Ok" : "unknown error");
        }
        int Code() const
        {
            return m_err;
        }
    protected:
        void Clear()
        {
            Set(0,NULL);
        }
        virtual void Set(int err, const char * errmsg)
        {
            if(m_errmsg)
                free(m_errmsg);
            m_err=err;
            m_errmsg = (char*)errmsg; /*? strdup(errmsg):NULL;*/
            if(m_errmsg)
                wxLogError(wxT("%s"),ConvA2W(errmsg).c_str());

        }
        friend class MusikDb;
    private:
        int m_err;
        char * m_errmsg;
        DbError(const DbError&);
        DbError &operator=(const DbError &);
    };
    class ResultBase
    {
    public:
        const DbError & Error() const 
        {
            return m_err;
        }
        DbError & Error()
        {
            return m_err;
        }

        friend class MusikDb;
    private:
        DbError m_err;
    };
    class ResultSet : public ResultBase
    {
    public:
        ResultSet()
        {
            m_cols = m_rows = 0;
            m_data = NULL;
        }
        virtual ~ResultSet()
        {
            if(m_data && m_xFreeData)
                m_xFreeData(m_data);

        }
        int Columns() const
        {
            return m_cols;
        }
        int Rows() const
        {
            return m_rows == 0 ? 0 : m_rows - 1;
        }
        const char * ColumnName(int c) const
        {
            if(!m_data || c >= m_cols)
            {
                return NULL;
            }
            return m_data[c];
        }
        const char * Value(int r,int c) const
        {
            if(!m_data || m_rows == 0 || r + 1 >= m_rows || c >= m_cols)
            {
                return NULL;
            }
            return m_data[(r+1) * m_cols + c]; //r+1 because r== 0 are the column names
        }
    protected:
        virtual void SetData(size_t rows,size_t cols,char ** data,void (*xFreeData)(char**))
        {
            if(m_data && m_xFreeData)
                m_xFreeData(m_data);
            m_xFreeData = xFreeData;
            m_data = data;
            m_rows = rows;
            m_cols = cols;
            
        }

        friend class MusikDb;
    private:
        void (*m_xFreeData)(char**);
        char ** m_data;
        int m_cols,m_rows;
    };
    class ResultCB : public ResultBase
    {
    public:
        typedef int (*tResultCB)(void* pT, int numCols, char **results, char ** columnNames);

        ResultCB(void* pT,tResultCB cb)
            :m_pT(pT),
            m_cb(cb)
        {
            
        }
        static int callback(void *pThis, int numCols, char **results, char ** columnNames)
        {
            ResultCB* pMyThis = (ResultCB*)pThis;
            return pMyThis->m_cb(pMyThis->m_pT,numCols,results,columnNames);
        }
    private:
        void* m_pT;
        tResultCB m_cb;
        friend class MusikDb;
    };
 
	MusikDb()
    {}
	virtual ~MusikDb()
    {
    }

    virtual MusikDb *Clone() = 0;
    virtual bool Open	(const wxString & dbfilename)=0;
	virtual void Close	() = 0;

    virtual void BeginTransaction()
	{
        Exec( "begin transaction;");
	}
	void EndTransaction()
	{ 
		Exec("end transaction;");
	}

    virtual bool SetCacheSize(int size)=0;
    
    class IBusyCallback
    {
    public:
        virtual bool OnBusy()=0;
        virtual ~IBusyCallback(){}
    };

    virtual void SetBusyHandler(IBusyCallback *pIBusy)
    {
        m_pIBusyCallback = pIBusy;   
    }
    virtual bool Exec(const char * q,ResultSet * pRes)=0;
    virtual bool Exec(const char * q,DbError *perr = NULL)=0;
    virtual bool Exec(const char * q,int * pInt,DbError *perr = NULL) = 0;
    virtual bool Exec(const char * q,double * pDbl,DbError *perr = NULL) = 0;
    virtual bool Exec(const char * q,ResultCB  & ResCb) = 0;
    virtual bool Exec(const char * q,wxString *pString,MusikDb::DbError *perr = NULL)=0;

protected:
    void SetResult(ResultSet * pRes,size_t rows,size_t cols,char ** data,void (*xFreeData)(char**))
    {
        pRes->SetData(rows,cols,data,xFreeData);
        pRes->Error().Clear();
    }
    void SetError(ResultSet *pRes,int err, const char * errmsg)
    {
        pRes->SetData(0,0,NULL,NULL);
        pRes->Error().Set(err,errmsg);
    }
    void ClearError(DbError *perr)
    {
        perr->Clear();
    }
    void SetError(DbError *perr,int err, const char * errmsg)
    {
        perr->Set(err,errmsg);
    }
    void SetError(ResultBase *pRes,int err, const char * errmsg)
    {
        pRes->Error().Set(err,errmsg);
    }
    bool CallBusyHandler()
    {
        if(!m_pIBusyCallback)
            return false;
        return m_pIBusyCallback->OnBusy();
    }
    private:
        IBusyCallback *m_pIBusyCallback;

};


#endif

/*
 *  MusikDb.h
 *
 *  Object which controls database manipulation.
 *
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
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
            if(m_errmsg)
                sqlite_freemem(m_errmsg);
        }
        const char *Text()
        {
            return m_errmsg ?  m_errmsg : "Ok";
        }
        int Code()
        {
            return m_err;
        }

    protected:
        virtual void Set(int err, char * errmsg)
        {
            if(m_errmsg)
                sqlite_freemem(m_errmsg);
            m_err=err;
            m_errmsg = errmsg;
            if(m_errmsg)
                wxLogError(wxT("%s"),ConvA2W(errmsg).c_str());

        }
        friend class MusikDb;
    private:
        int m_err;
        char * m_errmsg;
    };
    class ResultSet
    {
    public:
        ResultSet()
        {
            m_cols = m_rows = 0;
            m_data = NULL;
        }
        virtual ~ResultSet()
        {
            if(m_data)
                sqlite_free_table(m_data);

        }
        const DbError & Error() const 
        {
            return m_err;
        }
    protected:
        virtual void SetData(size_t rows,size_t cols,char ** data)
        {
            if(m_data)
                sqlite_free_table(m_data);
            m_data = data;
            m_rows = rows;
            m_cols = cols;
            
        }
        DbError & Error()
        {
            return m_err;
        }

        friend class MusikDb;
    private:
        DbError m_err;
        char ** m_data;
        int m_cols,m_rows;
    };
 
	MusikDb();
	~MusikDb();


    virtual bool Open	(const wxString & dbfilename)=0;
	virtual void Close	() = 0;

    void BeginTransaction()
	{
        DbError err;
        Exec( "begin transaction;" ,&err);
	}
	void EndTransaction()
	{ 
        DbError err;
		Exec("end transaction;",&err);
	}

    virtual bool SetCacheSize(int size);

    virtual bool Exec(const char * q,ResultSet * pRes)=0;
    virtual bool Exec(const char * q,DbError *perr)=0;
    virtual bool Exec(const char * q,int * pInt,DbError *perr) = 0;
    virtual bool Exec(const char * q,double * pDbl,DbError *perr) = 0;

protected:
    void SetResult(ResultSet * pRes,size_t rows,size_t cols,char ** data)
    {
        pRes->SetData(rows,cols,data);
        pRes->Error().Set(SQLITE_OK,NULL);
    }
    void SetError(ResultSet *pRes,int err, char * errmsg)
    {
        pRes->SetData(0,0,NULL);
        pRes->Error().Set(err,errmsg);
    }
    void SetError(DbError *perr,int err, char * errmsg)
    {
        perr->Set(err,errmsg);
    }
	mutable wxCriticalSection m_csDBAccess; // to lock all accesses to m_pDB. 
									// used instead of wxMutex, because this is faster on windows. on linux 
									// a wxMutex is used automatically instead


};

struct sqlite;

class MusikDb_Sqlite : public MusikDb
{
public:

    MusikDb_Sqlite();
    ~MusikDb_Sqlite();


    virtual bool Open	(const wxString & dbfilename);
    virtual void Close	();


    virtual bool SetCacheSize(int size)
    {
        DbError err;
        return Exec(QueryString("PRAGMA cache_size = %d;",size),&err);
    }

    virtual bool Exec(const char * q,ResultSet * pRes);
    virtual bool Exec(const char * q,DbError *perr);
    virtual bool Exec(const char * q,int * pInt,DbError *perr);
    virtual bool Exec(const char * q,double * pDbl,DbError *perr);

private:
    sqlite * m_pDB;

protected:
    void CreateDBFuncs();
    static void remprefixFunc(sqlite_func *context, int argc, const char **argv);
    static void cnvISO8859_1ToUTF8Func(sqlite_func *context, int argc, const char **argv);
    static void wxjuliandayFunc(sqlite_func *context, int argc, const char **argv);
    static void cnvMusikOldDTFormatToJuliandayFunc(sqlite_func *context, int argc, const char **argv);
    static void fuzzycmpFunc(sqlite_func *context, int argc, const char **argv);				
    static int sqlite_callback(void *args, int numCols, char **results, char ** columnNames);

};

#endif

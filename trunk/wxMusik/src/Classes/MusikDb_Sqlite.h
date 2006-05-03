/*
*  MusikDb.h
*
*  Concrete Class which controls database manipulation using sqlite2 db.
*
*  
*  Copyright (c) 2006 Gunnar Roth (gunnar.roth@gmx.de)
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_DB_SQLITE_H
#define MUSIK_DB_SQLITE_H

#include "MusikDb.h"

class MusikDb_Sqlite : public MusikDb
{
public:

    MusikDb_Sqlite()
    {
        m_pDB = NULL;
    }
    ~MusikDb_Sqlite()
    {
        InternalClose();
    }

    virtual MusikDb* Clone()
    {
        MusikDb * pDb= new MusikDb_Sqlite();
        if(!pDb)
            return NULL;
        if(pDb->Open(m_DbName))
            return pDb;
        delete pDb;
        return NULL;
    }

    virtual bool Open	(const wxString & dbfilename);
    virtual void Close	()
    {
        InternalClose();
    }

    virtual bool SetCacheSize(int size)
    {
        DbError err;
        return Exec(QueryString("PRAGMA cache_size = %d;",size),&err);
    }
    virtual void SetBusyHandler(IBusyCallback *pIBusy)
    {
        MusikDb::SetBusyHandler(pIBusy);
        if(pIBusy)
            sqlite_busy_handler(m_pDB,busycallback,this);
        else
            sqlite_busy_timeout(m_pDB,20000);
    }

    virtual bool Exec(const char * q,ResultSet * pRes);
    virtual bool Exec(const char * q,DbError *perr );
    virtual bool Exec(const char * q,int * pInt,DbError *perr)
    {
        wxString n;
        bool bRes = Exec(q,&n,perr);
        *pInt = wxStringToInt(n);
        return bRes;
    }
    virtual bool Exec(const char * q,double * pDbl,DbError *perr)
    {
        wxString n;
        bool bRes = Exec(q,&n,perr);
        *pDbl = StringToDouble(n);
        return bRes;

    }
    virtual bool Exec(const char * q,wxString *pString,MusikDb::DbError *perr);

    virtual bool Exec(const char * q,ResultCB  & ResCb);

private:
    sqlite * m_pDB;
    wxString m_DbName;
protected:
    void InternalClose();
    void CreateDBFuncs();
    static void remprefixFunc(sqlite_func *context, int argc, const char **argv);
    static void cnvISO8859_1ToUTF8Func(sqlite_func *context, int argc, const char **argv);
    static void wxjuliandayFunc(sqlite_func *context, int argc, const char **argv);
    static void cnvMusikOldDTFormatToJuliandayFunc(sqlite_func *context, int argc, const char **argv);
    static void fuzzycmpFunc(sqlite_func *context, int argc, const char **argv);				
    static int busycallback(void *pThis, const char *,int /*cnt*/)
    {
        MusikDb_Sqlite & This = *(MusikDb_Sqlite*)pThis;
        return This.CallBusyHandler() ? 1:0;
    }
};
#endif

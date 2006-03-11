/*
*  MusikDb_Sqlite3.h
*
*  Concrete Class which controls database manipulation using sqlite3 db.
*
*  
*  Copyright (c) 2006 Gunnar Roth (gunnar.roth@gmx.de)
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#ifndef MUSIK_DB_SQLITE3_H
#define MUSIK_DB_SQLITE3_H

#include "MusikDb.h"
#include "sqlite3.h"

class MusikDb_Sqlite3 : public MusikDb
{
public:

    MusikDb_Sqlite3()
    {
        m_pDB = NULL;
    }
    ~MusikDb_Sqlite3()
    {
        InternalClose();
    }

    virtual MusikDb* Clone()
    {
        MusikDb * pDb= new MusikDb_Sqlite3();
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
            sqlite3_busy_handler(m_pDB,busycallback,this);
        else
            sqlite3_busy_timeout(m_pDB,20000);
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
        *pDbl = CharStringToDouble(ConvW2A(n));
        return bRes;

    }
    virtual bool Exec(const char * q,wxString *pString,MusikDb::DbError *perr);

    virtual bool Exec(const char * q,ResultCB  & ResCb);

private:
    sqlite3 * m_pDB;
    wxString m_DbName;
protected:
    void InternalClose();
    void CreateDBFuncs();
    static void remprefixFunc(sqlite3_context *context, int argc, sqlite3_value **argv);
    static void fuzzycmpFunc(sqlite3_context *context, int argc, sqlite3_value **argv);				
    static int busycallback(void *pThis,int /*cnt*/)
    {
        MusikDb_Sqlite3 & This = *(MusikDb_Sqlite3*)pThis;
        return This.CallBusyHandler() ? 1:0;
    }
};
#endif
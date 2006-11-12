/*
*  MusikDb_Sqlite3.cpp
*
*  Class which controls database manipulation.
*
*  Uses: SQLite3
*  
*  Copyright (c) 2006 Gunnar Roth (gunnar.roth@gmx.de)
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
//--- wx ---//
#include "myprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "MusikDb_Sqlite3.h"

#include "3rd-Party/Bitap/libbitap.h"


bool MusikDb_Sqlite3::Open	(const wxString & dbfilename)
{
    m_DbName = dbfilename;

    if( sqlite3_open( ConvToUTF8(dbfilename), &m_pDB) != SQLITE_OK )
    {
        
        wxLogError(wxT("%s"),ConvA2W(sqlite3_errmsg(m_pDB)).c_str());
        sqlite3_close(m_pDB);
        m_pDB = NULL;
        return false;
    }  
    wxLogNull lognull;
    CreateDBFuncs();
    sqlite3_busy_timeout(m_pDB,20000);
    return true;
}

void MusikDb_Sqlite3::InternalClose	()
{
    if(m_pDB)
        sqlite3_close(m_pDB);
    m_pDB = NULL;
}

bool MusikDb_Sqlite3::Exec(const char * q,MusikDb::ResultSet * pRes)
{
    wxASSERT(m_pDB);
    char ** result = NULL;
    char *  errmsg = NULL;
    int nRow, nCol;
    int e = sqlite3_get_table(m_pDB,q,&result,&nRow,&nCol,&errmsg);
    if(e == SQLITE_OK)
    {
        SetResult(pRes,nRow,nCol,result,sqlite3_free_table);
        return true;
    }
    SetError(pRes,e,errmsg);
    sqlite3_free(errmsg);
    return false;
}

bool MusikDb_Sqlite3::Exec(const char * q,MusikDb::DbError *perr)
{
    DbError myerr;
    if(!perr)
        perr = &myerr;
    wxASSERT(m_pDB);
    char *  errmsg = NULL;
    int e = sqlite3_exec(m_pDB,q,NULL,NULL,&errmsg);
    if(e == SQLITE_OK)
    {
        ClearError(perr);
        return true;
    }
    SetError(perr,e,errmsg);
    sqlite3_free(errmsg);
    return false;

}

bool MusikDb_Sqlite3::Exec(const char * q,wxString *pString,MusikDb::DbError *perr)
{
    DbError myerr;
    if(!perr)
        perr = &myerr;
    //--- run query ---//
    const char *pTail;
    sqlite3_stmt *pStmt;
    if(pString) 
        pString->Empty();
    int e = sqlite3_prepare( m_pDB, q,strlen(q),&pStmt, &pTail  );
    if(e != SQLITE_OK)
    {
        SetError(perr,e,sqlite3_errmsg(m_pDB));
        return false;
    }
    bool bRet = true;
    //--- look and see if there's one row ---//
    if ( (e = sqlite3_step( pStmt )) == SQLITE_ROW )
    {
        if(pString) 
        {   
            const char * col = (const char*)sqlite3_column_text(pStmt,0);
            *pString = ConvFromUTF8( col ? col : "");
        }
    }
    else /*if(e != SQLITE_DONE)*/
    {
        bRet = false;
    }

    //--- close up ---//
    
    if((e = sqlite3_finalize( pStmt )) != SQLITE_OK)
    {
        SetError(perr,e,sqlite3_errmsg(m_pDB));
        return false;
    }
    ClearError(perr);
    return bRet;
}

bool MusikDb_Sqlite3::Exec(const char * q,MusikDb::ResultCB  & ResCb)
{
    wxASSERT(m_pDB);
    char *errmsg = NULL;
    int e = sqlite3_exec(m_pDB,q,ResCb.callback,&ResCb,&errmsg);
    if(e == SQLITE_OK)
    {
        return true;
    }
    SetError(&ResCb,e,errmsg);
    sqlite3_free(errmsg);
    return false;
}

void MusikDb_Sqlite3::CreateDBFuncs()
{
    wxASSERT(m_pDB);
    static const struct {
        char *zName;
        signed char nArg;
        unsigned char argType;           /* 0: none.  1: db  2: (-1) */
        unsigned char eTextRep;          /* 1: UTF-16.  0: UTF-8 */
        unsigned char needCollSeq;
        void (*xFunc)(sqlite3_context*,int,sqlite3_value **);
    } aFuncs[] = 
    {
        { "remprefix",   1, 0,SQLITE_UTF8,0, remprefixFunc},
        { "fuzzycmp",    3, 0,SQLITE_UTF8,0, fuzzycmpFunc },
    };
    /*  static struct {
    char *zName;
    int nArg;
    int dataType;
    void (*xStep)(sqlite3_func*,int,const char**);
    void (*xFinalize)(sqlite3_func*);
    } aAggs[] = {
    { "min",    1, 0,              minStep,      minMaxFinalize },
    { "max",    1, 0,              maxStep,      minMaxFinalize },
    { "sum",    1, sqlite3_NUMERIC, sumStep,      sumFinalize    },
    { "avg",    1, sqlite3_NUMERIC, sumStep,      avgFinalize    },
    { "count",  0, sqlite3_NUMERIC, countStep,    countFinalize  },
    { "count",  1, sqlite3_NUMERIC, countStep,    countFinalize  },
    };
    */
    size_t i;

    for(i=0; i<sizeof(aFuncs)/sizeof(aFuncs[0]); i++)
    {
        void *pArg = 0;
        switch( aFuncs[i].argType )
        {
        case 1: pArg = m_pDB; break;
        case 2: pArg = (void *)(-1); break;
        }

        sqlite3_create_function(m_pDB, aFuncs[i].zName,aFuncs[i].nArg,aFuncs[i].eTextRep, pArg, 
            aFuncs[i].xFunc, 0,0);
    }
    /* 
    for(i=0; i<sizeof(aAggs)/sizeof(aAggs[0]); i++){
    sqlite3_create_aggregate(m_pDB, aAggs[i].zName,
    aAggs[i].nArg, aAggs[i].xStep, aAggs[i].xFinalize, 0);
    sqlite3_function_type(m_pDB, aAggs[i].zName, aAggs[i].dataType);
    }
    */
}
void MusikDb_Sqlite3::remprefixFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    const char *z = (const char *)sqlite3_value_text(argv[0]);
    if( argc<1 || z ==0 ) return;
    //	char * pPrefixArray[] = sqlite3_user_data(context);
    static const char * pPrefixArray[] =
    {
        "The ",
            "Der ",
            "Die ",
            "Das ",
            "Le ",
            "La "
    };
    int zlen = strlen(z);
    for(size_t i=0; i<sizeof(pPrefixArray)/sizeof(pPrefixArray[0]); i++)
    {
        int prefixlen = strlen(pPrefixArray[i]);
        if(strncasecmp(pPrefixArray[i],z,prefixlen) == 0)
        {
            if(zlen >= prefixlen)
            {
                sqlite3_result_text(context, z + prefixlen, zlen - prefixlen,SQLITE_TRANSIENT);
                return;
            }
        }
    }
    sqlite3_result_text(context, z, zlen,SQLITE_TRANSIENT);
}

class MyBitap : public bitapType
{
public:
    MyBitap()
    {
    }
    ~MyBitap()
    {
        DeleteBitap(this);
    }
    bool Init(const unsigned char *Pattern)
    {
        return NewBitap(this,Pattern) >= 0;
    }
    const unsigned char *Find(const unsigned char *in, int inl, int e, int *ereturn = NULL,
        const unsigned char **bReturn = NULL)
    {
        return FindWithBitap(this,in,inl,e,ereturn,bReturn);
    }

    static void Destroy(void * me)
    {
        MyBitap * pThis = (MyBitap*)me;
        delete pThis;
    }
};


void MusikDb_Sqlite3::fuzzycmpFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    if( argc!=3) return;
    int AllowedErrors  = sqlite3_value_int(argv[2]);
    const unsigned char *Pattern = sqlite3_value_text(argv[0]);
    const unsigned char *Text = sqlite3_value_text(argv[1]);
    if(!Pattern || !Text)
        return;
    MyBitap *pBitap = (MyBitap *)sqlite3_get_auxdata(context,0);
    int Result = 1;
    if(!pBitap)
    {
        pBitap = new MyBitap;
        Result = pBitap->Init(Pattern) ? 1 : 0;
        if(Result)
            sqlite3_set_auxdata(context,0,pBitap,MyBitap::Destroy);
        else
            delete pBitap;
    }    
    if(Result)
    {
        Result = NULL != pBitap->Find(Text,strlen ((const char*)Text), AllowedErrors) ? 1 : 0;
    }
    sqlite3_result_int(context,Result);
}

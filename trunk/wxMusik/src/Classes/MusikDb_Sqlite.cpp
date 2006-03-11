/*
*  MusikDb_Sqlite.cpp
*
*  Class which controls database manipulation.
*
*  Uses: SQLite2
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
#include "MusikDb_Sqlite.h"
#include "sqlite.h"
#include "3rd-Party/Bitap/libbitap.h"


bool MusikDb_Sqlite::Open	(const wxString & dbfilename)
{
    m_DbName = dbfilename;
    char *errmsg = NULL;
    m_pDB = sqlite_open( wxConvertWX2MB(dbfilename), 0666, &errmsg );

    if( !m_pDB )
    {
        wxLogError(wxT("%s"),ConvA2W(errmsg).c_str());
        free(errmsg);
        return false;
    }  
    wxLogNull lognull;
    CreateDBFuncs();
    sqlite_busy_timeout(m_pDB,20000);
    return true;
}

void MusikDb_Sqlite::InternalClose	()
{
    if(m_pDB)
        sqlite_close(m_pDB);
    m_pDB = NULL;
}

bool MusikDb_Sqlite::Exec(const char * q,MusikDb::ResultSet * pRes)
{
    wxASSERT(m_pDB);
    char ** result = NULL;
    char *  errmsg = NULL;
    int nRow, nCol;
    int e = sqlite_get_table(m_pDB,q,&result,&nRow,&nCol,&errmsg);
    if(e == SQLITE_OK)
    {
        SetResult(pRes,nRow,nCol,result,sqlite_free_table);
        return true;
    }
    SetError(pRes,e,errmsg);
    return false;
}

bool MusikDb_Sqlite::Exec(const char * q,MusikDb::DbError *perr)
{
    DbError myerr;
    if(!perr)
        perr = &myerr;
    wxASSERT(m_pDB);
    char *  errmsg = NULL;
    int e = sqlite_exec(m_pDB,q,NULL,NULL,&errmsg);
    if(e == SQLITE_OK)
    {
        ClearError(perr);
        return true;
    }
    SetError(perr,e,errmsg);
    return false;

}

bool MusikDb_Sqlite::Exec(const char * q,wxString *pString,MusikDb::DbError *perr)
{
    DbError myerr;
    if(!perr)
        perr = &myerr;
    //--- run query ---//
    const char *pTail;
    sqlite_vm *pVM;
    if(pString) 
        pString->Empty();
    char *errmsg = NULL;
    int e = sqlite_compile( m_pDB, q, &pTail, &pVM, &errmsg );
    if(errmsg)
    {
        SetError(perr,e,errmsg);
        return false;
    }
    int numcols = 0;
    const char **coldata;
    const char **coltypes;
    bool bRet = true;
    //--- look and see if there's one row ---//
    if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
    {
        if(pString) 
            *pString = ConvFromUTF8(coldata[0] ? coldata[0] : "");
    }
    else
        bRet = false;
    //--- close up ---//
    e = sqlite_finalize( pVM, &errmsg );
    if(errmsg)
    {
        SetError(perr,e,errmsg);
        return false;
    }
    ClearError(perr);
    return bRet;
}

bool MusikDb_Sqlite::Exec(const char * q,MusikDb::ResultCB  & ResCb)
{
    wxASSERT(m_pDB);
    DbError myerr;
    char *errmsg = NULL;
    int e = sqlite_exec(m_pDB,q,ResCb.callback,&ResCb,&errmsg);
    if(e == SQLITE_OK)
    {
        return true;
    }
    SetError(&ResCb,e,errmsg);
    return false;
}

void MusikDb_Sqlite::CreateDBFuncs()
{
    wxASSERT(m_pDB);
    static struct 
    {
        char *zName;
        int nArg;
        int dataType;
        void (*xFunc)(sqlite_func*,int,const char**);
        void * userdata;
    } aFuncs[] = 
    {
        { "remprefix",      1, SQLITE_TEXT, remprefixFunc, 0 },
        { "cnvISO8859_1ToUTF8",     1, SQLITE_TEXT, cnvISO8859_1ToUTF8Func, 0 },
        { "wxjulianday", 1, SQLITE_TEXT, wxjuliandayFunc,0 },// for backward compatibility
        { "cnvMusikOldDTFormatToJulianday",	1, SQLITE_NUMERIC, cnvMusikOldDTFormatToJuliandayFunc, 0 },
        { "fuzzycmp",     3, SQLITE_TEXT, fuzzycmpFunc ,0},
    };
    /*  static struct {
    char *zName;
    int nArg;
    int dataType;
    void (*xStep)(sqlite_func*,int,const char**);
    void (*xFinalize)(sqlite_func*);
    } aAggs[] = {
    { "min",    1, 0,              minStep,      minMaxFinalize },
    { "max",    1, 0,              maxStep,      minMaxFinalize },
    { "sum",    1, SQLITE_NUMERIC, sumStep,      sumFinalize    },
    { "avg",    1, SQLITE_NUMERIC, sumStep,      avgFinalize    },
    { "count",  0, SQLITE_NUMERIC, countStep,    countFinalize  },
    { "count",  1, SQLITE_NUMERIC, countStep,    countFinalize  },
    };
    */
    size_t i;

    for(i=0; i<sizeof(aFuncs)/sizeof(aFuncs[0]); i++)
    {
        sqlite_create_function(m_pDB, aFuncs[i].zName,aFuncs[i].nArg, aFuncs[i].xFunc, aFuncs[i].userdata);
        if( aFuncs[i].xFunc )
        {
            sqlite_function_type(m_pDB, aFuncs[i].zName, aFuncs[i].dataType);
        }
    }
    /* 
    for(i=0; i<sizeof(aAggs)/sizeof(aAggs[0]); i++){
    sqlite_create_aggregate(m_pDB, aAggs[i].zName,
    aAggs[i].nArg, aAggs[i].xStep, aAggs[i].xFinalize, 0);
    sqlite_function_type(m_pDB, aAggs[i].zName, aAggs[i].dataType);
    }
    */
}
void MusikDb_Sqlite::cnvISO8859_1ToUTF8Func(sqlite_func *context, int argc, const char **argv)
{
    if( argc<1 || argv[0]==0 ) return;
    const wxCharBuffer buf = ConvFromISO8859_1ToUTF8(argv[0]);
    int len =strlen(buf);
    sqlite_set_result_string(context,buf, len);
}
void MusikDb_Sqlite::remprefixFunc(sqlite_func *context, int argc, const char **argv)
{
    if( argc<1 || argv[0]==0 ) return;
    //	char * pPrefixArray[] = sqlite_user_data(context);
    static const char * pPrefixArray[] =
    {
        "The ",
            "Der ",
            "Die ",
            "Das ",
            "Le ",
            "La "
    };
    int argvlen = strlen(argv[0]);
    for(size_t i=0; i<sizeof(pPrefixArray)/sizeof(pPrefixArray[0]); i++)
    {
        int prefixlen = strlen(pPrefixArray[i]);
        if(strncasecmp(pPrefixArray[i],argv[0],prefixlen) == 0)
        {
            if(argvlen >= prefixlen)
            {
                sqlite_set_result_string(context, argv[0]+ prefixlen, argvlen - prefixlen);
                return;
            }
        }
    }
    sqlite_set_result_string(context, argv[0], argvlen);
}
void MusikDb_Sqlite::cnvMusikOldDTFormatToJuliandayFunc(sqlite_func *context, int argc, const char **argv)
{
    if( argc<1 || argv[0]==0 ) return;
#ifdef __WXMSW__
    wxDateTime x;
    if( x.ParseFormat(ConvA2W(argv[0])))
    {
        sqlite_set_result_double(context, x.GetJulianDayNumber());
    }
#else
    struct tm tm;
    const char *result = strptime(argv[0], "%c", &tm);
    if ( !result )
        return;
    wxDateTime x(tm);
    sqlite_set_result_double(context, x.GetJulianDayNumber());
#endif
}
void MusikDb_Sqlite::wxjuliandayFunc(sqlite_func *context, int argc, const char **argv)
{
    if( argc<1 || argv[0]==0 ) return;
    sqlite_set_result_string(context,argv[0], strlen(argv[0]));
}

void MusikDb_Sqlite::fuzzycmpFunc(sqlite_func *context, int argc, const char **argv)
{
    if( argc!=3 || argv[0]==0 || argv[1]==0) return;
    int AllowedErrors  = argv[2]?atoi(argv[2]):0;
    const char *Pattern = argv[0];
    const char *Text =   argv[1];
    int Result = 0;
    bitapType b;
    if(NewBitap(&b,(const unsigned char*)Pattern)>=0)
    {
        if (NULL != FindWithBitap(&b, (const unsigned char*)Text,strlen (Text), AllowedErrors,NULL,NULL))
        {
            Result = 1;
        }
        DeleteBitap(&b);
    }
    sqlite_set_result_int(context,Result);
}

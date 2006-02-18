/*
*  MusikLibrary.cpp
*
*  Object which controls database manipulation.
*
*  Uses: SQLite
*  
*  Copyright (c) 2005 Gunnar Roth (gunnar.roth@gmx.de)
*
*  See the file "license.txt" for information on usage and redistribution
*  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "myprec.h"
#include "MusikDb.h"

#include "sqlite.h"


bool MusikDb_Sqlite::Open	(const wxString & dbfilename)
{
    char *errmsg = NULL;
    m_pDB = sqlite_open( wxConvertWX2MB(dbfilename), 0666, &errmsg );

    if( !m_pDB )
    {
        wxLogError(wxT("%s"),ConvA2W(errmsg).c_str());
        free(errmsg);
        return false;
    }    
   // CreateDBFuncs();
    return true;
}

void MusikDb_Sqlite::Close	()
{
    if(m_pDB)
        sqlite_close(m_pDB);
    m_pDB = NULL;
}

bool MusikDb_Sqlite::Exec(const char * q,MusikDb_Sqlite::ResultSet * pRes)
{
    wxASSERT(m_pDB);
    char ** result = NULL;
    char *  errmsg = NULL;
    int nRow, nCol;
    int e = sqlite_get_table(m_pDB,q,&result,&nRow,&nCol,&errmsg);
    if(e == SQLITE_OK)
    {
        SetResult(pRes,nRow,nCol,result);
        return true;
    }
    SetError(pRes,e,errmsg);
    return false;
}

bool MusikDb_Sqlite::Exec(const char * q,MusikDb_Sqlite::DbError *perr)
{
    wxASSERT(m_pDB);
    char *  errmsg = NULL;
    int e = sqlite_exec(m_pDB,q,NULL,NULL,&errmsg);
    if(e == SQLITE_OK)
    {
        SetError(perr,SQLITE_OK,NULL);
        return true;
    }
    SetError(perr,e,errmsg);
    return false;

}

bool MusikDb_Sqlite::Exec(const char * q,int * pInt,MusikDb_Sqlite::DbError *perr)
{
    //--- run query ---//
    const char *pTail;
    sqlite_vm *pVM;
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

    //--- look and see if there's one row ---//
    if ( sqlite_step( pVM, &numcols, &coldata, &coltypes ) == SQLITE_ROW )
    {
        *pInt = coldata[0] ? atoi( coldata[0] ) : 0;
    }
    //--- close up ---//
    e = sqlite_finalize( pVM, &errmsg );
    if(errmsg)
    {
        SetError(perr,e,errmsg);
        return false;
    }
    return true;
}
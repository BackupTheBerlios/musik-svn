/*
 *  MusikUtils.cpp
 *
 *  Global functions that don't really belong 
 *  anywhere but here.
 *  
 *  Copyright (c) 2003 Casey Langen (casey@bak.rr.com)
 *	Contributors: Simon Windmill, Dustin Carter, Gunnar Roth, Wade Brainerd
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

//--- For compilers that support precompilation, includes "wx/wx.h" ---//
#include "wx/wxprec.h"

//--- globals ---//
#include "MusikGlobals.h"
#include "MusikUtils.h"


//--- wx ---//
#include <wx/textfile.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include "wx/defs.h"
#include <wx/mimetype.h>
#include <mpeg/id3v1/id3v1genres.h>

#if wxUSE_STATLINE
#include "wx/statline.h"
#endif


class wxPlaylistTraverser : public wxDirTraverser
{
public:
    wxPlaylistTraverser(wxArrayString& files) : m_files(files) { }

    virtual wxDirTraverseResult OnFile(const wxString& filename)
    {
		wxFileName fn( filename );
		wxString ext = fn.GetExt();
		ext.MakeLower();

		if ( ext.Left(2) == wxT( "mp" ))
		{
			wxString add = fn.GetFullName();
			m_files.Add( add );
		}
        
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname))
    {
        return wxDIR_CONTINUE;
    }

private:
    wxArrayString& m_files;
};



void  DelimitStr( wxString sStr, wxString sDel, wxArrayString &aReturn, bool bRemoveDelimiter )
{
	aReturn.Clear();
	wxStringTokenizer toke( sStr, sDel );

	while( toke.HasMoreTokens() )
	{
		//--- if we remove the delimiter, do nothing ---//
		if ( bRemoveDelimiter )
			aReturn.Add( toke.GetNextToken() );
		
		//--- otherwise tack it to the end ---//
		else
			aReturn.Add( toke.GetNextToken() + sDel );
	}

	//--- strip last delimiter ---//
	if(aReturn.GetCount() > 0)
		aReturn.Item( aReturn.GetCount()-1 ).Replace( sDel, wxT( "" ), true );

	return;
}

void SortArrayByLength ( wxArrayString* pArray )
{
	wxString sTemp;

	for ( size_t i = 0; i < pArray->GetCount(); i++ )
	{
		for ( size_t j = 0; j < pArray->GetCount()-1; j++ )
		{	
			if( pArray->Item( j ).Length() < pArray->Item( j + 1 ).Length() )
			{
				sTemp = pArray->Item( j );
				pArray->Item( j + 0 ) = pArray->Item( j + 1 );
				pArray->Item( j + 1 ) = sTemp;
			}
		}
	}
}

//wxString GetGenre ( const wxString & sGenre ) 
//{ 
//	// if sGenre is a number, the name will be returned
//	// if sGenre is something like (nnn) XXXXX , the name of the number nnn will be returned
//	// if nothing matches the sGenre will be returned, but if sGenre is an empty string, wxT("<unknown>") will be returned
//	wxString aReturn(sGenre);
//	if ( sGenre.Length() > 0 )
//	{
//		int nGenreID = -1;
//		if(wxIsdigit(sGenre[0]))
//			nGenreID = wxStringToInt( sGenre );
//		else if (sGenre.StartsWith(wxT("("),&aReturn))
//		{	
//			aReturn = aReturn.BeforeFirst(')');
//			nGenreID = wxStringToInt( aReturn );
//		}
//		if (nGenreID >=0 && nGenreID < ID3_NR_OF_V1_GENRES )
//			aReturn = ConvA2W( ID3_v1_genre_description[nGenreID] );
//	}
//	return aReturn;
//}

int GetGenreID( const CSongMetaData::StringData & sGenre )
{		
	return TagLib::ID3v1::genreIndex(TagLib::String(sGenre));
//	for ( int i = 0; i < ID3_NR_OF_V1_GENRES; i++ )
//	{
//		if(strcasecmp(sGenre,ID3_v1_genre_description[i]) == 0)
//			return i;
//	}
//	return -1; //--- return -1 if unknown ---//
}

wxString MStoStr( int timems )
{
	wxString result;

	int ms = timems;
	int hours = ms / 1000 / 60 / 60;
	ms -= hours * 1000 * 60 * 60;
	int minutes = ms / 1000 / 60;
	ms -= minutes * 1000 * 60;
	int seconds = ms / 1000;

	if ( hours > 0 )
		result.sprintf( wxT("%d:%02d:%02d"), hours, minutes, seconds );
	else
		result.sprintf( wxT("%d:%02d"), minutes, seconds );
	
	return result;
}


void GetPlaylistDir( wxArrayString & aFiles )
{
	aFiles.Clear();

	wxDir dir( MUSIK_PLAYLIST_DIR );
	if ( dir.IsOpened() )
	{
		wxPlaylistTraverser traverser( aFiles );
		dir.Traverse( traverser );
	}

	return;
}

wxArrayString FileToStringArray(  const wxString & sName )
{
	wxArrayString aReturn;

	wxTextFile In( sName );

	if ( !wxFileExists( sName ) )
		return aReturn;

	else
	{
		if ( In.Open() )
		{
			for ( int i = 0; i < ( int )In.GetLineCount(); i++ )
			{
				wxString sCheck = In.GetLine( i );
				sCheck.Replace( wxT(" "), wxT(""), TRUE );
				if ( !sCheck.IsEmpty())
					aReturn.Add( In.GetLine( i ) );
			}
			In.Close();
		}
	}
	return aReturn;
}

int MusikRound(float x)
{
	//--- as far as I can tell this is equivalent to (int)(x+0.5) ;) ---//
    float i;
    i = x - (float)(int)x;
    if ((i < 0.5) && (i > -0.5))
		return (int)x;
    else 
		if (i >= 0.5)
			return (int)(x + 1.0);
		else return (int)(x - 1.0);
}

int	FindStrInArray( wxArrayString* array, wxString pattern )
{
	for ( size_t i = 0; i < array->GetCount(); i++ )
	{
		if ( array->Item( i ) == pattern )
			return i;
	}
	return -1;
}

void ReplaceNoCase( wxString& str, const wxString& oldstr, const wxString& newstr )
{
	int pos = 0;
	wxString strlow = str.Lower();
	wxString oldstrlow = oldstr.Lower();
	
	while ( ( pos = strlow.Find( oldstrlow ) ) != -1 )
	{
		str.Remove( pos, oldstr.Len() );
		strlow.Remove( pos, oldstr.Len() );
		str.insert( pos, newstr );
	}
}
wxString MoveArtistPrefixToEnd( const wxString & str )
{
	wxString Artist = str;
	const wxChar* Prefix = BeginsWithPreposition(Artist);
	if(Prefix)
	{
		Artist = Artist.Right(Artist.Len()-wxStrlen(Prefix));
		Artist += wxT(", ");
		Artist += Prefix;
	}
	return Artist;
}
const wxChar * BeginsWithPreposition( const wxString & s )
{
	static const wxChar * pPrefixArray[] =
	{
		wxT("The "),
			wxT("Der "),
			wxT("Die "),
			wxT("Das "),
			wxT("Le "),
			wxT("La ")
	};

	for(size_t i=0; i<sizeof(pPrefixArray)/sizeof(pPrefixArray[0]); i++)
	{
		int prefixlen = wxStrlen(pPrefixArray[i]);
		if(wxStrnicmp(pPrefixArray[i],s.c_str(),prefixlen) == 0)
		{
			if(s.size() >= (size_t)prefixlen)
			{
				return pPrefixArray[i];
			}
		}
	}
	return NULL;

}
wxString SanitizedString( const wxString & str )
{
	if ( !wxGetApp().Prefs.bBlankSwears )
		return str;

	wxString outstr( str );
	ReplaceNoCase( outstr, _("fuck"), 		wxT("****") );
	ReplaceNoCase( outstr, _("shit"), 		wxT("****") );
	ReplaceNoCase( outstr, _("tits"), 		wxT("****") );
	ReplaceNoCase( outstr, _("dick"), 		wxT("****") );
	ReplaceNoCase( outstr, _("bitch"), 		wxT("*****") );
	ReplaceNoCase( outstr, _("whore"), 		wxT("*****") );
	ReplaceNoCase( outstr, _("slut"), 		wxT("****") );
	ReplaceNoCase( outstr, _("wank"),		wxT("****") );
	ReplaceNoCase( outstr, _("cunt"), 		wxT("****") );
	ReplaceNoCase( outstr, _("manchester"), wxT("**********") );
	ReplaceNoCase( outstr, _("dildo"), 		wxT("*****") );

	return outstr;
}

void wxListCtrlSelAll( wxListCtrl *pList )
{
	#ifdef WXLISTCTRL_SETITEMSTATE_IS_BUGGY
		for( int i = 0; i < pList->GetItemCount(); i++ )
			pList->SetItemState( i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	#else
    		pList->SetItemState( -1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	#endif
}

void wxListCtrlSelNone( wxListCtrl *pList )
{
#ifdef WXLISTCTRL_SETITEMSTATE_IS_BUGGY
	for( int i = 0; i < pList->GetItemCount(); i++ )
		pList->SetItemState( i, 0, wxLIST_STATE_SELECTED );
#else
	pList->SetItemState( -1, 0, wxLIST_STATE_SELECTED );
#endif
}


wxString ColourToString ( const wxColour& color )
{
	return wxString::Format( wxT("%d, %d, %d"), color.Red(), color.Green(), color.Blue() );
}

wxColour StringToColour ( const wxString& string )
{
	wxStringTokenizer st( string, wxT(",") );

	unsigned long r = 0, g = 0, b = 0;
	bool ok;
	ok = st.GetNextToken().ToULong( &r );
	ok = ok && st.GetNextToken().ToULong( &g );
	ok = ok && st.GetNextToken().ToULong( &b );

	if ( ok )
		return wxColour( r, g, b );
	else
		return wxColour( 0, 0, 0 );
}

wxString GetFramePlacement( wxFrame* frame )
{
	wxPoint pos = frame->GetPosition();
	wxSize size = frame->GetSize();
	bool maximized = frame->IsMaximized();
	bool iconized = frame->IsIconized();
	wxString s = wxString::Format( wxT("%d,%d,%d,%d,%d,%d"), pos.x, pos.y, size.x, size.y, maximized, iconized );
	return s;
}

bool SetFramePlacement( wxFrame* frame, wxString place )
{
	wxStringTokenizer st( place, wxT(",") );

	unsigned long x=0, y=0, w=0, h= 0, maximized, iconized;
	bool ok;
	ok = st.GetNextToken().ToULong( &x );
	ok = ok && st.GetNextToken().ToULong( &y );
	ok = ok && st.GetNextToken().ToULong( &w );
	ok = ok && st.GetNextToken().ToULong( &h );
	ok = ok && st.GetNextToken().ToULong( &maximized );
	ok = ok && st.GetNextToken().ToULong( &iconized );

	ok = ok && (x+w <= (unsigned long)wxSystemSettings::GetMetric( wxSYS_SCREEN_X )) && (y+h <= (unsigned long)wxSystemSettings::GetMetric( wxSYS_SCREEN_Y ));
	if ( ok )
	{
		frame->SetSize( wxRect( x, y, w, h ) );
#if 0 //--- still working out the issues with this, lots of nasty event order probs ---//
		if ( maximized ) 
			frame->Maximize();
		else if ( iconized )
			frame->Iconize();
#endif
	}
	return ok;
}

//------------------------------------------- Random numbers support -------------------------------------//

/* 
A C-program for MT19937, with initialization improved 2002/2/10.
Coded by Takuji Nishimura and Makoto Matsumoto.
This is a faster version by taking Shawn Cokus's optimization,
Matthe Bellew's simplification, Isaku Wada's real version.

Before using, initialize the state by using init_genrand(seed) 
or init_by_array(init_key, key_length).

Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.                          

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The names of its contributors may not be used to endorse or promote 
products derived from this software without specific prior written 
permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Any feedback is very welcome.
http://www.math.keio.ac.jp/matumoto/emt.html
email: matumoto@math.keio.ac.jp
*/

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

static unsigned long state[N]; /* the array for the state vector  */
static int left = 1;
static int initf = 0;
static unsigned long *next;

/* initializes state[N] with a seed */
void SeedRandom(unsigned long s)
{
	int j;
	state[0]= s & 0xffffffffUL;
	for (j=1; j<N; j++) {
		state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j); 
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array state[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		state[j] &= 0xffffffffUL;  /* for >32 bit machines */
	}
	left = 1; initf = 1;
}

static void next_state(void)
{
	unsigned long *p=state;
	int j;

	/* if init_genrand() has not been called, */
	/* a default initial seed is used         */
	if (initf==0) SeedRandom(5489UL);

	left = N;
	next = state;

	for (j=N-M+1; --j; p++) 
		*p = p[M] ^ TWIST(p[0], p[1]);

	for (j=M; --j; p++) 
		*p = p[M-N] ^ TWIST(p[0], p[1]);

	*p = p[M-N] ^ TWIST(p[0], state[0]);
}

/* generates a random number on [0,0x7fffffff]-interval */
long GetRandomNumber(void)
{
	unsigned long y;

	if (--left == 0) next_state();
	y = *next++;

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return (long)(y>>1);
}

wxLongLong GetTotalFilesize(const CMusikSongArray &songs)
{
	wxLongLong filesize = 0;
	for ( size_t i = 0; i < songs.GetCount(); i++ )
	{
		const CMusikSong &song = songs.Item( i );
		filesize += song.MetaData.nFilesize;
	}

	return filesize;
}

///////////////////////////////////////////////////////////////////////////////

CNiceFilesize::CNiceFilesize()
{
	m_Bytes = 0;
	m_Kilobytes = 0;
	m_Megabytes = 0;
	m_Gigabytes = 0;
	m_Terabytes = 0;
}

CNiceFilesize::~CNiceFilesize()
{
}

void CNiceFilesize::AddB( long b )
{
	if ( ( m_Bytes + b ) > 1023 )
	{
		long kb = ( m_Bytes + b ) / 1024;
		long by = ((m_Bytes + b) * 100 / 1024) % 100;
		AddK( kb );
		m_Bytes = by;
	}
	else
		m_Bytes += b;

}

void CNiceFilesize::AddK( long k )
{
	if ( ( m_Kilobytes + k ) > 1023 )
	{
		long mb = ( m_Kilobytes + k ) / 1024;
		long kb = ((m_Kilobytes + k) * 100 / 1024) % 100;
		AddM( mb );
		m_Kilobytes = kb;
	}
	else
		m_Kilobytes += k;

}

void CNiceFilesize::AddM( long m )
{
	if ( ( m_Megabytes + m ) > 1023 )
	{
		long gb = (m_Megabytes + m) / 1024;
		long mb = ((m_Megabytes + m) * 100 / 1024) % 100;
		AddG( gb );
		m_Megabytes = mb;
	}
	else
		m_Megabytes += m;

}

void CNiceFilesize::AddG( long g )
{	
	if ( ( m_Gigabytes + g ) > 1023 )
	{
		long tb = ( m_Gigabytes + g ) / 1024;
		long gb = ((m_Gigabytes + g) * 100 / 1024) % 100;
		AddT( tb );
		m_Gigabytes = gb;
	}
	else
		m_Gigabytes += g;
}


void CNiceFilesize::AddT( long t )
{
	m_Terabytes += t;
}

wxString CNiceFilesize::GetFormatted()
{
	if ( m_Terabytes > 0 )
		return wxString::Format( _("%d.%02d TB"), m_Terabytes, m_Gigabytes );
	
	if ( m_Gigabytes > 0 )
		return wxString::Format( _("%d.%02d GB"), m_Gigabytes, m_Megabytes );

	if ( m_Megabytes > 0 )
		return wxString::Format( _("%d.%02d MB"), m_Megabytes, m_Kilobytes );

	if ( m_Kilobytes > 0 )
		return wxString::Format( _("%d.%02d KB"), m_Kilobytes, m_Bytes );

	return wxString( _("Empty") );
}

CMusikTagger::CMusikTagger(const wxString &sTheMask, bool bConvertUnderscoresToSpaces)
:m_bConvertUnderscoresToSpaces(bConvertUnderscoresToSpaces)
{
	wxString sMask(sTheMask);
	sMask.Trim();
	if(sMask.Left(1) != wxFileName::GetPathSeparator())
	{
		sMask.Prepend( wxFileName::GetPathSeparator() );
	}
	wxRegEx reMatchRegexSpecialChars(wxT("([{}\\:\\^\\*\\.\\+\\$\\(\\)\\|\\?\\\\]|\\[|\\])"));
	reMatchRegexSpecialChars.ReplaceAll(&sMask,wxT("\\\\\\1"));// replace all special regex chars by \char

	wxRegEx reMatchPlaceHolder(wxT("%([[:alnum:]])"));
	wxString sMaskParse = sMask;
	while(reMatchPlaceHolder.Matches(sMaskParse))
	{
		size_t start=0, len=0;
		reMatchPlaceHolder.GetMatch(&start,&len,1);
		int nPlaceHolder = sMaskParse.Mid(start,len).c_str()[0];
		m_PlaceHolderArray.Add(nPlaceHolder);
		sMaskParse = sMaskParse.Right(sMaskParse.Length() - (start + len));
	}
	wxRegEx reMatchPlaceHolderRemoveTracknum(wxT("(%6|%n)"));// tracknum
	reMatchPlaceHolderRemoveTracknum.ReplaceAll(&sMask,wxT("([[:digit:]]+)"));

	wxRegEx reMatchPlaceHolderRemoveTracknumUC(wxT("(%N)"));// tracknum ( eat up surrounding spaces)
	reMatchPlaceHolderRemoveTracknumUC.ReplaceAll(&sMask,wxT("\\ *([[:digit:]]+)\\ *"));
	// replace all other %x by ([^\\/]+)
	wxRegEx reMatchPlaceHolderRemove(wxT("(%[a-z0-9])"));
	reMatchPlaceHolderRemove.ReplaceAll(&sMask,wxT("([^\\\\/]+)"));
	// replace all other %X by "\ *([^\\\\/]+)\ *" this eats up all surrounding spaces
	wxRegEx reMatchPlaceHolderRemoveUC(wxT("(%[A-Z])"));
	reMatchPlaceHolderRemoveUC.ReplaceAll(&sMask,wxT("\\ *([^\\\\/]+)\\ *"));

	sMask += '$';
	m_reMask.Compile(sMask);
}

bool CMusikTagger::Retag(CMusikSong * Song) const
{
	wxString sFile	= Song->MetaData.Filename.GetFullPath();
	//-------------------------------------------------//
	//--- get rid of the file extension.		---//
	//-------------------------------------------------//
	wxStripExtension(sFile);
	if(m_bConvertUnderscoresToSpaces)
		sFile.Replace(wxT( "_" ), wxT( " " ));

	if(m_reMask.Matches(sFile))
	{
		for(size_t i = 0;i < m_PlaceHolderArray.GetCount(); i++ )
		{
			
			size_t start = 0,len = 0;
			if(!m_reMask.GetMatch(&start,&len,i + 1))
				break;
			CSongMetaData::StringData sField = ConvToUTF8(sFile.Mid(start, len)).data();

			switch ( m_PlaceHolderArray[i] )
			{
			case '1':
			case 't':
			case 'T':
				Song->MetaData.Title = sField;
				break;
			case '2':
			case 'a':
			case 'A':
				Song->MetaData.Artist = sField;
				break;
			case '3':
			case 'b':
			case 'B':
				Song->MetaData.Album = sField;
				break;
			case '4':
			case 'g':
			case 'G':
				Song->MetaData.Genre = sField;
				break;
			case '5':
			case 'y':
			case 'Y':
				Song->MetaData.Year = sField;
				break;
			case '6':
			case 'n':
			case 'N':
				Song->MetaData.nTracknum = atoi( sField);
				break;
			default:
				// skip
				break;
			}

		}
		return true;
	}
	else
	{
		return false;
	}
}

/* (stolen from sqlite source)
** The string z[] is an ascii representation of a real number.
** Convert this string to a double.
**
** This routine assumes that z[] really is a valid number.  If it
** is not, the result is undefined.
**
** This routine is used instead of the library atof() function because
** the library atof() might want to use "," as the decimal point instead
** of "." depending on how locale is set.  But that would cause problems
** for SQL.  So this routine always uses "." regardless of locale.
*/
#define LONGDOUBLE_TYPE long double

double CharStringToDouble(const char *z){
	if(z == NULL)
		return 0.0;
	int sign = 1;
	LONGDOUBLE_TYPE v1 = 0.0;
	if( *z=='-' ){
		sign = -1;
		z++;
	}else if( *z=='+' ){
		z++;
	}
	while( wxIsdigit(*z) ){
		v1 = v1*10.0 + (*z - '0');
		z++;
	}
	if( *z=='.' ){
		LONGDOUBLE_TYPE divisor = 1.0;
		z++;
		while( wxIsdigit(*z) ){
			v1 = v1*10.0 + (*z - '0');
			divisor *= 10.0;
			z++;
		}
		v1 /= divisor;
	}
	if( *z=='e' || *z=='E' ){
		int esign = 1;
		int eval = 0;
		LONGDOUBLE_TYPE scale = 1.0;
		z++;
		if( *z=='-' ){
			esign = -1;
			z++;
		}else if( *z=='+' ){
			z++;
		}
		while( wxIsdigit(*z) ){
			eval = eval*10 + *z - '0';
			z++;
		}
		while( eval>=64 ){ scale *= 1.0e+64; eval -= 64; }
		while( eval>=16 ){ scale *= 1.0e+16; eval -= 16; }
		while( eval>=4 ){ scale *= 1.0e+4; eval -= 4; }
		while( eval>=1 ){ scale *= 1.0e+1; eval -= 1; }
		if( esign<0 ){
			v1 /= scale;
		}else{
			v1 *= scale;
		}
	}
	return sign<0 ? -v1 : v1;
}
/*
** Some powers of 64.  These constants are needed in the
** DoubleToCharString() routine below.
*/
#define _64e3  (64.0 * 64.0 * 64.0)
#define _64e4  (64.0 * 64.0 * 64.0 * 64.0)
#define _64e15 (_64e3 * _64e4 * _64e4 * _64e4)
#define _64e16 (_64e4 * _64e4 * _64e4 * _64e4)
#define _64e63 (_64e15 * _64e16 * _64e16 * _64e16)
#define _64e64 (_64e16 * _64e16 * _64e16 * _64e16)

/*
** The following procedure converts a double-precision floating point
** number into a string.  The resulting string has the property that
** two such strings comparied using strcmp() or memcmp() will give the
** same results as a numeric comparison of the original floating point
** numbers.
**
** This routine is used to generate database keys from floating point
** numbers such that the keys sort in the same order as the original
** floating point numbers even though the keys are compared using
** memcmp().
**
** The calling function should have allocated at least 14 characters
** of space for the buffer z[].
*/
void DoubleToCharString(double r, char *z){
	int neg;
	int exp;
	int cnt = 0;

	/* This array maps integers between 0 and 63 into base-64 digits.
	** The digits must be chosen such at their ASCII codes are increasing.
	** This means we can not use the traditional base-64 digit set. */
	static const char zDigit[] = 
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"|~";
	if( r<0.0 ){
		neg = 1;
		r = -r;
		*z++ = '-';
	} else {
		neg = 0;
		*z++ = '0';
	}
	exp = 0;

	if( r==0.0 ){
		exp = -1024;
	}else if( r<(0.5/64.0) ){
		while( r < 0.5/_64e64 && exp > -961  ){ r *= _64e64;  exp -= 64; }
		while( r < 0.5/_64e16 && exp > -1009 ){ r *= _64e16;  exp -= 16; }
		while( r < 0.5/_64e4  && exp > -1021 ){ r *= _64e4;   exp -= 4; }
		while( r < 0.5/64.0   && exp > -1024 ){ r *= 64.0;    exp -= 1; }
	}else if( r>=0.5 ){
		while( r >= 0.5*_64e63 && exp < 960  ){ r *= 1.0/_64e64; exp += 64; }
		while( r >= 0.5*_64e15 && exp < 1008 ){ r *= 1.0/_64e16; exp += 16; }
		while( r >= 0.5*_64e3  && exp < 1020 ){ r *= 1.0/_64e4;  exp += 4; }
		while( r >= 0.5        && exp < 1023 ){ r *= 1.0/64.0;   exp += 1; }
	}
	if( neg ){
		exp = -exp;
		r = -r;
	}
	exp += 1024;
	r += 0.5;
	if( exp<0 ) return;
	if( exp>=2048 || r>=1.0 ){
		strcpy(z, "~~~~~~~~~~~~");
		return;
	}
	*z++ = zDigit[(exp>>6)&0x3f];
	*z++ = zDigit[exp & 0x3f];
	while( r>0.0 && cnt<10 ){
		int digit;
		r *= 64.0;
		digit = (int)r;
		assert( digit>=0 && digit<64 );
		*z++ = zDigit[digit & 0x3f];
		r -= digit;
		cnt++;
	}
	*z = 0;
}

wxString GetForbiddenChars(wxPathFormat format)
{
	// Inits to forbidden characters that are common to (almost) all platforms.
	wxString strForbiddenChars = wxT("*?");

	switch ( wxFileName::GetFormat(format) )
	{
	default :
		wxFAIL_MSG( wxT("Unknown path format") );
		// !! Fall through !!

	case wxPATH_UNIX:
		break;

	case wxPATH_MAC:
		// On a Mac even names with * and ? are allowed (Tested with OS
		// 9.2.1 and OS X 10.2.5)
		strForbiddenChars = wxEmptyString;
		break;

	case wxPATH_DOS:
		strForbiddenChars += wxT("\\/:\"<>|");
		break;

	case wxPATH_VMS:
		break;
	}

	return strForbiddenChars;
}

void ReplaceChars(wxString &s,const wxString &chars,wxChar replaceby)
{
	for(size_t j = 0; j < s.Len();j++)
	{
		for(size_t i = 0; i < chars.Len();i++)
		{
			if(s[j] == chars[i])
				s[j] = replaceby;
		}
	}			

}
wxString SecToStr( int nSec )
{
	wxString result;

	int ms = nSec * 1000;
	int hours = ms / 1000 / 60 / 60;
	ms -= hours * 1000 * 60 * 60;
	int minutes = ms / 1000 / 60;
	ms -= minutes * 1000 * 60;
	int seconds = ms / 1000;

	if ( hours > 0 )
		result.sprintf( wxT( "%d:%02d:%02d" ), hours, minutes, seconds );
	else
		result.sprintf( wxT( "%d:%02d" ), minutes, seconds );

	return result;
}

bool GetFileTypeAssociationInfo(const wxString &sExt,wxString*psDescription,bool * pbAssociatedWithApp)
{
	*pbAssociatedWithApp = false;
	
	wxFileType *pFT = wxTheMimeTypesManager->GetFileTypeFromExtension(sExt);
	if(!pFT)
		return false;
	if(psDescription)
		pFT->GetDescription(psDescription);	 // use description from filetype.
	*pbAssociatedWithApp = FileTypeIsAssociated(*pFT);
	delete pFT;
	return true;
}
bool FileTypeIsAssociated(const wxFileType &ft)
{
	wxString appname = wxGetApp().GetAppName();
	wxString sCommmand = ft.GetOpenCommand(wxEmptyString).Lower();
	return sCommmand.Contains(appname.Lower());// if open command contains our appname, it is associated with us
}

void AssociateWithFileType(const wxString &sExt,const wxString &sDescription)
{
	wxFileType *pFT = wxTheMimeTypesManager->GetFileTypeFromExtension(sExt);
	wxString sMimetype;
	if(pFT)
	{
		pFT->GetMimeType(&sMimetype);
		delete pFT;
	}

	wxString sAppName = wxString::Format(wxT("\"%s\""),wxGetApp().argv[0]);
	

    wxFileTypeInfo fti(sMimetype,sAppName,wxT(""),sDescription,sExt.c_str(),NULL);
	wxString sShortDesc=wxT("wxMusik ")+sExt + wxT(" File");
#ifdef __WXMSW__
	fti.SetShortDesc(sShortDesc);
	fti.SetIcon(sAppName);
#endif
	pFT = wxTheMimeTypesManager->Associate(fti);
	delete pFT; // really weird api... Associate returns a wxFileType, which i don't need, but must delete
}
void UnassociateWithFileType(const wxString &sExt)
{
	wxFileType *pFT = wxTheMimeTypesManager->GetFileTypeFromExtension(sExt);
	if(pFT && FileTypeIsAssociated(*pFT))   //only unassociate, if file is associated with us
	{
		pFT->Unassociate();	
	}	
	delete pFT;
}
// MusikLogWindow
// -----------
BEGIN_EVENT_TABLE(MusikLogWindow, wxEvtHandler)
  EVT_MENU			( MUSIK_LOGWINDOW_NEWMSG,		MusikLogWindow::OnNewMsg)
END_EVENT_TABLE()

MusikLogWindow::MusikLogWindow(wxFrame *pParent,
						 const wxChar *szTitle,
						 long style)
						 :wxLogWindow(pParent,szTitle,(style & MUSIK_LW_ShowInitial) != 0,(style & MUSIK_LW_DoPass) != 0)
						 ,m_Style(style)
{
}



void MusikLogWindow::DoLogString(const wxChar *szString, time_t t)
{

	
/*
** on linux the Show from another thread
** ( if wxLogWarning is issued from a thread) 
** lead to a xlib fault and crash.
** same for mac oxs.
** 
** so we post a message to ourself.
**
**As i have expereience some crashes on gtk and osx ( osx is being deadlocked)
** i do the actual logging in the message handler
*/
	wxCommandEvent Evt	( wxEVT_COMMAND_MENU_SELECTED, MUSIK_LOGWINDOW_NEWMSG );
	Evt.SetString(szString);
	Evt.SetInt((int)t);
	wxPostEvent( this,Evt );
}

void MusikLogWindow::OnNewMsg(wxCommandEvent &evt)
{
	wxLogWindow::DoLogString(evt.GetString(),(time_t)evt.GetInt());
	if(m_Style & MUSIK_LW_ShowOnLog)
		Show(TRUE);
}
bool MusikLogWindow::OnFrameClose(wxFrame * frame)
{
	if(m_Style & MUSIK_LW_ClearContentOnClose)
	{
		wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED,wxID_CLEAR);
		frame->ProcessEvent(ev);
	}
	// allow to close
	return TRUE;
}

static const int wxID_TEXT = 3000;

BEGIN_EVENT_TABLE(wxMultiLineTextEntryDialog, wxDialog)
EVT_BUTTON(wxID_OK, wxMultiLineTextEntryDialog::OnOK)
END_EVENT_TABLE()

IMPLEMENT_CLASS(wxMultiLineTextEntryDialog, wxDialog)

wxMultiLineTextEntryDialog::wxMultiLineTextEntryDialog(wxWindow *parent,
						   const wxString& message,
						   const wxString& caption,
						   const wxString& value ,
						   long style ,
						   const wxPoint& pos ,const wxSize & size )
	:wxDialog(parent, -1, caption, pos, size,wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER | wxDIALOG_MODAL)
	, m_value(value)
{
	m_dialogStyle = style;
	m_value = value;

	wxBeginBusyCursor();

	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

	// 1) text message
	topsizer->Add( CreateTextSizer( message ), 0, wxALL, 10 );

	// 2) text ctrl
	m_textctrl = new wxTextCtrl(this, wxID_TEXT, value,
		wxDefaultPosition, wxSize(400, 80),
		(style & ~wxTextEntryDialogStyle)| wxTE_MULTILINE );
	topsizer->Add( m_textctrl, 1, wxEXPAND | wxLEFT|wxRIGHT, 15 );

#if wxUSE_VALIDATORS
	wxTextValidator validator( wxFILTER_NONE, &m_value );
	m_textctrl->SetValidator( validator );
#endif
	// wxUSE_VALIDATORS

#if wxUSE_STATLINE
	// 3) static line
	topsizer->Add( new wxStaticLine( this, -1 ), 0, wxEXPAND | wxLEFT|wxRIGHT|wxTOP, 10 );
#endif

	// 4) buttons
	topsizer->Add( CreateButtonSizer( style ), 0, wxCENTRE | wxALL, 10 );

	SetAutoLayout( TRUE );
	SetSizer( topsizer );

	topsizer->SetSizeHints( this );
	topsizer->Fit( this );

	Centre( wxBOTH );

	m_textctrl->SetSelection(-1, -1);
	m_textctrl->SetFocus();

	wxEndBusyCursor();
}


void wxMultiLineTextEntryDialog::OnOK(wxCommandEvent& WXUNUSED(event) )
{
#if wxUSE_VALIDATORS
	if( Validate() && TransferDataFromWindow() ) 
	{
		EndModal( wxID_OK );
	}
#else
	m_value = m_textctrl->GetValue();

	EndModal(wxID_OK);
#endif
	// wxUSE_VALIDATORS
}

void wxMultiLineTextEntryDialog::SetValue(const wxString& val)
{
	m_value = val;

	m_textctrl->SetValue(val);
}

#if wxUSE_VALIDATORS
void wxMultiLineTextEntryDialog::SetTextValidator( long style )
{
	wxTextValidator validator( style, &m_value );
	m_textctrl->SetValidator( validator );
}

void wxMultiLineTextEntryDialog::SetTextValidator( wxTextValidator& validator )
{
	m_textctrl->SetValidator( validator );
}

#endif
// wxUSE_VALIDATORS

BEGIN_EVENT_TABLE(wxStaticText_NoFlicker,wxStaticText)
EVT_ERASE_BACKGROUND(wxStaticText_NoFlicker::OnEraseBackground)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxCheckBox_NoFlicker,wxCheckBox)
EVT_ERASE_BACKGROUND(wxCheckBox_NoFlicker::OnEraseBackground)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxChoice_NoFlicker,wxChoice)
EVT_ERASE_BACKGROUND(wxChoice_NoFlicker::OnEraseBackground)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxButton_NoFlicker,wxButton)
EVT_ERASE_BACKGROUND(wxButton_NoFlicker::OnEraseBackground)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxTextCtrl_NoFlicker,wxWindow)
EVT_ERASE_BACKGROUND(wxTextCtrl_NoFlicker::OnEraseBackground)
END_EVENT_TABLE()


IMPLEMENT_CLASS(wxGenericIntValidator, wxGenericValidator)

#ifdef __WXMSW__
#include <Shlwapi.h>
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	HINSTANCE hinstDll;
	DWORD dwVersion = 0;

	/* For security purposes, LoadLibrary should be provided with a 
	fully-qualified path to the DLL. The lpszDllName variable should be
	tested to ensure that it is a fully qualified path before it is used. */
	hinstDll = LoadLibrary(lpszDllName);

	if(hinstDll)
	{
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
			"DllGetVersion");

		/* Because some DLLs might not implement this function, you
		must test for it explicitly. Depending on the particular 
		DLL, the lack of a DllGetVersion function can be a useful
		indicator of the version. */

		if(pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			HRESULT hr;

			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr))
			{
				dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}
	return dwVersion;
}
#endif //__WXMSW__

/* This code was copied from winamp mpc plugin v.98                 */
/* copyright Andree Buschmann, Andree.Buschmann@web.de              */
/* license of the code is LPGL						                */
/* modified by gunnar roth (gunnar.roth@gmx.de) Nov 10. 2004		*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "idtag.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#define strcasecmp	stricmp 
#else
#include <strings.h>
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

// split inpath to  path(with trailing /) and filename   and extension
void splitpath (const char *inpath, char *outpath, char *outname, char * ext) 
{
	if(outpath)
		outpath[0] = 0;
	if(outname)
		outname[0] = 0;
	if(ext)
		ext[0] = 0;
	char * extdot = strrchr(inpath,'.');
	if(extdot)
	{
		if(ext)
			strcpy(ext,extdot + 1);
	}
	
	char * lastslash = strrchr(inpath,'/');
	if(lastslash)
	{
		if(outpath)
		{
			strncpy(outpath, inpath, lastslash - inpath + 1);
			outpath[lastslash - inpath + 1] = 0;
		}
		if(outname)
		{
			if(extdot)
			{
				strncpy(outname, lastslash + 1,extdot - (lastslash + 1));
				outname[extdot - (lastslash + 1)]=0;
			}
			else
				strcpy(outname, lastslash + 1);
		}
	}
	else
	{
		if(outname)
			strcpy(outname, inpath);
	}
	
}

size_t   unicodeToUtf8 ( const wchar_t* lpWideCharStr, char* lpMultiByteStr, int cwcChars );
size_t   utf8ToUnicode ( const char* lpMultiByteStr, wchar_t* lpWideCharStr, int cmbChars );
int   ConvertANSIToUTF8 ( const char* ansi, char* utf8 );                   // convert current locale  to UTF-8


const char ListSeparator[] = { "; " };

// list of possible naming schemes
// the order is important, first match is used
// T: Title
// A: Artist
// L: Album
// C: Comment
// N: Track
// Y: Year
// G: Genre
// X: Discard

const char *SchemeList[] = {
    "%A/%L (%Y)/%A - %N %T",
    "%A/%L (%Y)/%A - %N_%T",
    "%L -- [%N] -- %A -- %T",
    "%L/[%N] -- %A -- %T",
    "%L -- [%N] %A -- %T",
    "%L/[%N] %A -- %T",
    "%A -- %L -- [%N] %T",
    "%A/%L -- [%N] %T",
    "%A/%L/[%N] - %T",
    "%A/%L/[%N] %T",
    "%L/%A -- %T",
    "%A_-_%L_-_%Y/%N_-_%A_-_%T",
    "%A_-_%L-%Y/%N_-_%A_-_%T",
    "%A_-_%L/%N_-_%A_-_%T",
    "%L-%Y/%N_-_%A_-_%T",
    "%L/%N_-_%A_-_%T",
    "%A_%L/%A_%N_%T",
    "%L/%A_%N_%T",
    "%L/%N_%A - %T",
    "%A_%L/%A_%T",
    "%L/%A_%T",
    "(%Y) %L/%A - %L - [%N] - %T",
    "(%Y) %L/%A - %L - (%N) - %T",
    "(%Y) %L/%A - %L - %N - %T",
    "(%Y) %L/%A - %L [%N] - %T",
    "(%Y) %L/%A - %L (%N) - %T",
    "(%Y) %L/%A - %L - [%N] %T",
    "(%Y) %L/%A - %L - (%N) %T",
    "(%Y) %L/%A - %L [%N] %T",
    "(%Y) %L/%A - %L (%N) %T",
    "%L (%Y)/%L - [%N] - %A - %T",
    "%L (%Y)/%L - (%N) - %A - %T",
    "%L (%Y)/%L - %N - %A - %T",
    "%L (%Y)/%L [%N] - %A - %T",
    "%L (%Y)/%L (%N) - %A - %T",
    "%L (%Y)/%L - [%N] %A %T",
    "%L (%Y)/%L - (%N) %A %T",
    "%L (%Y)/%L [%N] %A - %T",
    "%L (%Y)/%L (%N) %A - %T",
    "%A - %L - [%N] - %T",
    "%A - %L - (%N) - %T",
    "%A - %L - %N - %T",
    "%A - %L [%N] - %T",
    "%A - %L (%N) - %T",
    "%A - %L - [%N] %T",
    "%A - %L - (%N) %T",
    "%A - %L [%N] %T",
    "%A - %L (%N) %T",
    "%A - %L/%N - %A - %T",
    "%A - %L/%A - %N - %T",
    "%A - %L/%N- %A - %T",
    "%L/%A - [%N] - %T",
    "%L/%A - (%N) - %T",
    "%L/%A - [%N] %T",
    "%L/%A - (%N) %T",
    "%L/%A [%N] - %T",
    "%L/%A (%N) - %T",
    "%L/%A (%N) %T",
    "%L/%A [%N] %T",
    "%L/%N - %A - %T",
    "%L/%A - %N - %T",
    "%L/%N- %A - %T",
    "%A - %L/%N %A - %T",
    "%L/%N %A - %T",
    "%A - %L/%N - %T",
    "%A/%L/%N - %T",
    "%A - %L/%A - %T",
    "%L/%A - %T",
    "%A/%L/%N. %T",
    "%A/%L/%N-%T",
    "%A/%L/%N.%T",
    "%A/%L/%N - %L - %T",
	

};

/* V A R I A B L E S */
const char* const GenreList [] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
    "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
    "Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
    "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
    "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
    "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
    "Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
    "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
    "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
    "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
    "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
    "New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
    "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
    "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
    "Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
    "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
    "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
    "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
    "Po""rn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
    "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
    "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
    "Goa", "Drum & Bass", "Club House", "Ha""rd""co""re", "Terror",
    "Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
    "Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
    "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
    "SynthPop",
};

// copy value from tag field to dest, returns length
size_t CSimpleTagReader::CopyTagValue ( char* dest, const char* item, size_t count )
{
    size_t  i;

    for ( i = 0; i < tagitem_count; i++ ) {
        // Are items case sensitive?
        if ( strcasecmp( item, tagitems[i].Item ) == 0 ) {
            if ( count > tagitems[i].ValueSize + 1 ) count = tagitems[i].ValueSize + 1;
            memcpy ( dest, tagitems[i].Value, count );
            return count;
        }
    }

    return 0;
}

// return pointer to value in tag field
char* CSimpleTagReader::TagValue ( const char* item)
{
    size_t  i;

    for ( i = 0; i < tagitem_count; i++ ) {
        // Are items case sensitive?
        if ( strcasecmp ( item, tagitems[i].Item ) == 0 )
            return (char*)tagitems[i].Value;
    }

    return 0;
}

// add new field to tagdata
int CSimpleTagReader::NewTagField ( const char* item, const size_t itemsize, const char* value, const size_t valuesize, const unsigned int flags,  const enum tag_rel reliability )
{
    size_t itemnum = tagitem_count++;

    // no real error handling yet
    tagitems = (TagItem*) realloc ( tagitems, tagitem_count * sizeof (TagItem) );
    if ( tagitems == NULL ) {
        tagitem_count = 0;
        return 1;
    }
    tagitems[itemnum].Item  = (char*)malloc ( itemsize  + 1 );
    if ( tagitems[itemnum].Item == NULL ) {                           // couldn't allocate memory, revert to original items
        tagitems = (TagItem*) realloc ( tagitems, --tagitem_count * sizeof (TagItem) );
        return 1;
    }
    tagitems[itemnum].Value = (char*)malloc ( valuesize + 1 );
    if ( tagitems[itemnum].Value == NULL ) {                          // couldn't allocate memory, revert to original items
        free ( tagitems[itemnum].Item );
        tagitems = (TagItem*) realloc ( tagitems, --tagitem_count * sizeof (TagItem) );
        return 1;
    }
    memcpy ( tagitems[itemnum].Item , item , itemsize  );
    memcpy ( tagitems[itemnum].Value, value, valuesize );

    tagitems[itemnum].Item [itemsize]  = '\0';
    tagitems[itemnum].Value[valuesize] = '\0';

    tagitems[itemnum].ItemSize         = itemsize;
    tagitems[itemnum].ValueSize        = valuesize;
    tagitems[itemnum].Flags            = flags;
    tagitems[itemnum].Reliability      = reliability;

    return 0;
}

// replace old value in tagdata field
int CSimpleTagReader::ReplaceTagField ( const char* value, const size_t valuesize, const unsigned int flags, size_t itemnum, const enum tag_rel reliability )
{
    // no real error handling yet
    free ( tagitems[itemnum].Value );
    tagitems[itemnum].Value = (char*)malloc ( valuesize + 1 );
    if ( tagitems[itemnum].Value == NULL )                            // couldn't allocate memory
        return 1;
    memcpy ( tagitems[itemnum].Value, value, valuesize );

    tagitems[itemnum].Value[valuesize] = '\0';
    tagitems[itemnum].ValueSize        = valuesize;
    tagitems[itemnum].Flags            = flags;
    tagitems[itemnum].Reliability      = reliability;
    return 0;
}

// insert item to tagdata. calls either new or replace
int CSimpleTagReader::InsertTagField ( const char* item, size_t itemsize, const char* value, size_t valuesize, const unsigned int flags)
{
    size_t  i;

    if ( itemsize  == 0 ) itemsize  = strlen ( item  );                     // autodetect size
    if ( valuesize == 0 ) valuesize = strlen ( value );                     // autodetect size

    for ( i = 0; i < tagitem_count; i++ ) {
        // are items case sensitive?
        if ( strcasecmp ( item, tagitems[i].Item ) == 0 )               // replace value of first item found
            return ReplaceTagField ( value, valuesize, flags, i, (tag_rel)0 );
    }

    return NewTagField ( item, itemsize, value, valuesize, flags, (tag_rel)0 );// insert new field
}

// insert item to tagdata but only if reliability > previous tag's reliability
int CSimpleTagReader::InsertTagFieldLonger ( const char* item, size_t itemsize, const char* value, size_t valuesize, const unsigned int flags, const enum tag_rel reliability )
{
    size_t  i;

    if ( itemsize  == 0 ) itemsize  = strlen ( item  );                     // autodetect size
    if ( valuesize == 0 ) valuesize = strlen ( value );                     // autodetect size

    for ( i = 0; i < tagitem_count; i++ ) {
        // are items case sensitive?
        if ( strcasecmp ( item, tagitems[i].Item ) == 0 ) {
            if ( reliability > tagitems[i].Reliability )              // replace value of first item found
                return ReplaceTagField ( value, valuesize, flags, i, reliability );
            else
                return 0;
        }
    }

    return NewTagField ( item, itemsize, value, valuesize, flags, reliability );  // insert new field
}

// Convert UNICODE to UTF-8
// Return number of bytes written
size_t unicodeToUtf8 ( const wchar_t * lpWideCharStr, char* lpMultiByteStr, int cwcChars )
{
    const wchar_t*   pwc = lpWideCharStr;
    unsigned char*   pmb = (unsigned char*)lpMultiByteStr;
    const wchar_t*   pwce;
    size_t  cBytes = 0;

    if ( cwcChars >= 0 ) {
        pwce = pwc + cwcChars;
    } else {
        pwce = (wchar_t *)((size_t)-1);
    }

    while ( pwc < pwce ) {
        wchar_t  wc = *pwc++;

        if ( wc < 0x00000080 ) {
            *pmb++ = (char)wc;
            cBytes++;
        } else
        if ( wc < 0x00000800 ) {
            *pmb++ = (char)(0xC0 | ((wc >>  6) & 0x1F));
            cBytes++;
            *pmb++ = (char)(0x80 |  (wc        & 0x3F));
            cBytes++;
        } else
        /*if ( wc < 0x00010000 )*/ {
            *pmb++ = (char)(0xE0 | ((wc >> 12) & 0x0F));
            cBytes++;
            *pmb++ = (char)(0x80 | ((wc >>  6) & 0x3F));
            cBytes++;
            *pmb++ = (char)(0x80 |  (wc        & 0x3F));
            cBytes++;
        }
        if ( wc == L'\0' )
            return cBytes;
    }

    return cBytes;
}

// Convert UTF-8 coded string to UNICODE
// Return number of characters converted
size_t utf8ToUnicode ( const char* lpMultiByteStr, wchar_t* lpWideCharStr, int cmbChars )
{
    const unsigned char*    pmb = (const unsigned char*)lpMultiByteStr;
    wchar_t	*			    pwc = lpWideCharStr;
    const unsigned char*	pmbe;
    size_t  cwChars = 0;

    if ( cmbChars >= 0 ) {
        pmbe = pmb + cmbChars;
    } else {
        pmbe = (unsigned char*)((size_t)-1);
    }

    while ( pmb < pmbe ) {
        char            mb = *pmb++;
        unsigned int    cc = 0;
        unsigned int    wc;

        while ( (cc < 7) && (mb & (1 << (7 - cc)))) {
            cc++;
        }

        if ( cc == 1 || cc > 6 )                    // illegal character combination for UTF-8
            continue;

        if ( cc == 0 ) {
            wc = mb;
        } else {
            wc = (mb & ((1 << (7 - cc)) - 1)) << ((cc - 1) * 6);
            while ( --cc > 0 ) {
                if ( pmb == pmbe )                  // reached end of the buffer
                    return cwChars;
                mb = *pmb++;
                if ( ((mb >> 6) & 0x03) != 2 )      // not part of multibyte character
                    return cwChars;
                wc |= (mb & 0x3F) << ((cc - 1) * 6);
            }
        }

        if ( wc & 0xFFFF0000 )
            wc = L'?';
        *pwc++ = (wchar_t)(wc & 0x0000FFFF);
        cwChars++;
        if ( wc == L'\0' )
            return cwChars;
    }

    return cwChars;
}

// convert Windows ANSI to UTF-8
int ConvertANSIToUTF8 ( const char* ansi, char* utf8 )
{
    wchar_t*  wszValue;          // Unicode value
    int  ansi_len;
    int  len;

    *utf8 = '\0';
    if ( ansi == NULL )
        return 0;

    ansi_len = strlen ( ansi );

    if ( (wszValue = (wchar_t *)malloc ( (ansi_len + 1) * sizeof(wchar_t) )) == NULL )
        return -1;

    // Convert ANSI value to Unicode
    if ((len = mbstowcs( wszValue,ansi, ansi_len + 1)) == -1 ) {
        free ( wszValue );
        return -1;
    }

    // Convert Unicode value to UTF-8
    if ((len = unicodeToUtf8 ( wszValue, utf8, -1 )) == 0 ) {
        free ( wszValue );
        return -1;
    }

    free ( wszValue );

    return len;
}


int CSimpleTagReader::ConvertTagFieldToUTF8 ( size_t itemnum )
{
    char*   uszValue;          // UTF-8 value
    int  len;
    int     error;

    if ( tagitems[itemnum].Flags & 1<<1 )                             // data in binary
        return 0;

    if ( tagitems[itemnum].Value     == NULL ||                       // nothing to do
         tagitems[itemnum].ValueSize == 0 )
        return 0;

    uszValue = (char  *)malloc ( (tagitems[itemnum].ValueSize + 1) * 3 );
    if ( uszValue == NULL ) {
        return -1;
    }

    // Convert ANSI value to Unicode
	
    if ((len =  ConvertANSIToUTF8(tagitems[itemnum].Value,uszValue)) == -1) {
        free ( uszValue );
        return -1;
    }

    error = ReplaceTagField ( uszValue, len-1, tagitems[itemnum].Flags, itemnum, tagitems[itemnum].Reliability );

    free ( uszValue );

    return error;
}



// replace list separator characters in tag field
int CSimpleTagReader::ReplaceListSeparator ( const char* old_sep, const char* new_sep, size_t itemnum )
{
    unsigned char*  new_value;
    unsigned char*  p;
    size_t          os_len;
    size_t          ns_len;
    size_t          count;
    size_t          new_len;
    size_t          i;
    int             error;

    if ( tagitems[itemnum].Flags & 1<<1 )                             // data in binary
        return 0;

    os_len = strlen ( old_sep );
    ns_len = strlen ( new_sep );
    if ( os_len == 0 ) os_len = 1;                                          // allow null character
    if ( ns_len == 0 ) ns_len = 1;

    if ( tagitems[itemnum].Value     == NULL ||                       // nothing to do
         tagitems[itemnum].ValueSize == 0 )
        return 0;

    count = 0;
    for ( i = 0; i < tagitems[itemnum].ValueSize - os_len + 1; i++ ) {
        if ( memcmp ( tagitems[itemnum].Value+i, old_sep, os_len ) == 0 )
            count++;
    }

    if ( count == 0 )
        return 0;

    new_len = tagitems[itemnum].ValueSize - (count * os_len) + (count * ns_len);
    if ( (new_value = (unsigned char *)malloc ( new_len )) == NULL )
        return 1;

    p = new_value;
    for ( i = 0; i < tagitems[itemnum].ValueSize; i++ ) {
        if ( i + os_len - 1 >= tagitems[itemnum].ValueSize ||
             memcmp ( tagitems[itemnum].Value+i, old_sep, os_len ) != 0 ) {
            *p++ = tagitems[itemnum].Value[i];
        } else {
            memcpy ( p, new_sep, ns_len );
            p += ns_len;
            i += os_len - 1;
        }
    }

    error = ReplaceTagField ( (const char*)new_value, new_len, tagitems[itemnum].Flags, itemnum, tagitems[itemnum].Reliability );

    free ( new_value );

    return error;
}

int
GenreToInteger ( const char* GenreStr)
{
    size_t  i;

    for ( i = 0; i < sizeof(GenreList) / sizeof(*GenreList); i++ ) {
        if ( 0 == strcasecmp ( GenreStr, GenreList [i] ) )
            return i;
    }

    return -1;
}

void
GenreToString ( char* GenreStr, const unsigned int genre )
{
    GenreStr [0] = '\0';
    if ( genre < NO_GENRES )
        strcpy ( GenreStr, GenreList [genre] );
}

/* F U N C T I O N S */

/*
 *  Copies src to dst. Copying is stopped at `\0' char is detected or if
 *  len chars are copied.
 *  Trailing blanks are removed and the string is `\0` terminated.
 */

static void
memcpy_crop ( char* dst, const char* src, size_t len )
{
    size_t  i;

    for ( i = 0; i < len; i++ )
        if  ( src[i] != '\0' )
            dst[i] = src[i];
        else
            break;

    // dst[i] points behind the string contents
    while ( i > 0  &&  dst [i-1] == ' ' )
        i--;

    dst [i] = '\0';
}

// replaces % sequences with real characters
void fix_percentage_sequences ( char* string )
{
    char temp[_MAX_PATH];
    char* t = temp;
    char* s = string;
    int value;
    int b1, b2; //, b3, b4;
    int v1, v2; //, v3, v4;

    do {
        value = *s++;

        if ( value == '%' ) {
            if ( *s != '\0' )   b1 = *s++;
            else                b1 = '\0';
            if ( *s != '\0' )   b2 = *s++;
            else                b2 = '\0';

            if ( ((b1 >= '0' && b1 <= '9') || (b1 >= 'A' && b1 <= 'F')) &&
                 ((b2 >= '0' && b2 <= '9') || (b2 >= 'A' && b2 <= 'F')) ) {

                if ( b1 <= '9' )  v1 = b1 - '0';
                else              v1 = b1 - 'A' + 10;
                if ( b2 <= '9' )  v2 = b2 - '0';
                else              v2 = b2 - 'A' + 10;

                if ( v1 == 0 && v2 == 0 ) {         // %00xx
                    /*
                    if ( *s != '\0' )   b1 = *s++;
                    else                b1 = '\0';
                    if ( *s != '\0' )   b2 = *s++;
                    else                b2 = '\0';
                    if ( *s != '\0' )   b3 = *s++;
                    else                b2 = '\0';
                    if ( *s != '\0' )   b4 = *s++;
                    else                b4 = '\0';

                    if ( b1 <= '9' )  v1 = b1 - '0';
                    else              v1 = b1 - 'A' + 10;
                    if ( b2 <= '9' )  v2 = b2 - '0';
                    else              v2 = b2 - 'A' + 10;
                    if ( b3 <= '9' )  v3 = b3 - '0';
                    else              v3 = b3 - 'A' + 10;
                    if ( b4 <= '9' )  v4 = b4 - '0';
                    else              v4 = b4 - 'A' + 10;

                    if ( v1 != 0 || v2 != 0 ) {
                        *t++ = ' ';                 // no multibyte support, unknown character
                    } else {                        // %0000+xxxx+
                        *t++ = ' ';

                        while ( *s && ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F')) )
                            s++;
                    }
                    */

                    *t++ = ' ';

                    while ( *s && ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F')) )
                        s++;
                } else {                            // %xx
                    *t++ = (char)((v1 << 4) + v2);
                }
            } else {                                // %aa
                *t++ = '%';
                *t++ = (char)b1;
                *t++ = (char)b2;
            }
        } else {
            *t++ = (char) value;
        }
    } while ( value != '\0' );

    strcpy ( string, temp );
}

// gets tag information from filename
int CSimpleTagReader::GuessTagFromName ( const char* filename, const char* naming_scheme)
{
    char scheme  [_MAX_PATH];

    char name    [_MAX_PATH];
    char dir     [_MAX_DIR];
    char fname   [_MAX_FNAME];

    char Title   [_MAX_PATH];
    char Artist  [_MAX_PATH];
    char Album   [_MAX_PATH];
    char Track   [_MAX_PATH];
    char Genre   [_MAX_PATH];
    char Year    [_MAX_PATH];
    char Comment [_MAX_PATH];
    char discard [_MAX_PATH];


    char *dest = NULL;      // destination
    char *sep  = NULL;      // field separator
    unsigned int s_pos = 0; // position in scheme
    unsigned int f_pos = 0; // position in filename
    unsigned int d_len = 0; // length of destination
    unsigned int s_len = 0; // length of source
    unsigned long len;      // length of name
    int paths = 0;
    unsigned int i;

    strcpy ( name, filename );
    for ( i = 0; i < strlen (name) - 5; i++ ) {
#ifdef _WIN32
		if( name[i] == '\\')
		{
			name[i] = '/';
		}
#endif
        if (name[i] != '/' ) continue;
        if ( strncasecmp ( (char *)(name+i+1), "(CD", 3 ) == 0 || strncasecmp ( (char *)(name+i+1), "(DVD", 4 ) == 0 )
            name[i] = ' ';
		
    }

    splitpath( name, dir, fname ,NULL);

    if ( strlen ( fname ) == 0 || strlen (naming_scheme) >= _MAX_PATH )
        return 1;

    for ( i = 0; i <= strlen (naming_scheme); i++ ) {
        char c = naming_scheme[i];
        if ( c == '/' ) {
            if ( scheme[i + 1] == '/' ) i++;
            paths++;
        }
        scheme[i] = c;
    }

    if ( paths > 0 ) {
        int path_pos = strlen ( dir ) - 1;

        while ( paths > 0 && path_pos >= 0 ) {
            if ( dir[--path_pos] == '/' ) paths--;
        }
        path_pos++;

        strcpy ( name, (char *)(dir+path_pos) );
        strcat ( name, fname );
        len = strlen ( name );
    } else {
        strcpy ( name, fname );
        len = strlen ( name );
    }

    Title  [0] = '\0';
    Artist [0] = '\0';
    Album  [0] = '\0';
    Track  [0] = '\0';
    Genre  [0] = '\0';
    Year   [0] = '\0';
    Comment[0] = '\0';

    for ( s_pos = 0; s_pos < strlen (scheme) + 1; s_pos++ ) {
        bool bIsNumber = false;
        if ( scheme[s_pos] == '%' ) {
            s_pos++;
            sep = (char *)(scheme+s_pos+1);
            s_len = 0;
            while ( (sep < (char *)(scheme+strlen(scheme)+1)) && ((*(sep+s_len) != '%' && *(sep+s_len) != '\0')) ) s_len++;
            if ( s_len == 0 ) s_len = 1;

            switch ( scheme[s_pos] ) {
            case 't':   // title
            case 'T':
                dest  = Title;
                d_len = sizeof (Title);
                break;
            case 'a':   // artist
            case 'A':
                dest  = Artist;
                d_len = sizeof (Artist);
                break;
            case 'l':   // album
            case 'L':
                dest  = Album;
                d_len = sizeof (Album);
                break;
            case 'c':   // comment
            case 'C':
                dest  = Comment;
                d_len = sizeof (Comment);
                break;
            case 'n':   // track
            case 'N':
                bIsNumber = true;
                dest  = Track;
                d_len = sizeof (Track);
                break;
            case 'y':   // year
            case 'Y':
                bIsNumber = true;
                dest  = Year;
                d_len = sizeof (Year);
                break;
            case 'g':   // genre
            case 'G':
                dest  = Genre;
                d_len = sizeof (Genre);
                break;
            case 'x':   // disrecard
            case 'X':
                dest  = discard;
                d_len = sizeof (discard);
                break;
            default:
                dest  = NULL;
                d_len = 0;
                break;
            }
        }

        if ( dest != NULL && sep != NULL ) {
            char *bak = dest;
            char *end = (char *)(dest + d_len);

            while ( f_pos < len && strncmp ((char*)(name+f_pos), sep, s_len) != 0 ) {
                if(bIsNumber && !isdigit(name[f_pos]))
                    break;
                if ( dest < end ) {
                    char c = name[f_pos++];
                    if ( c == '_' ) c = ' ';
                    if ( c == '\'' && f_pos < len && name[f_pos] == '\'' ) {    // '' -> "
                        f_pos++;
                        c = '"';
                    }
                    if ( c == ' ' && f_pos < len && name[f_pos] == ' ' ) {      // '  ' -> ' '
                        f_pos++;
                    }
                    *dest++ = c;
                } else {
                    f_pos++;
                }
            }

            if ( dest < end ) *dest++ = '\0';
            while ( dest != bak && *dest == ' ' ) *dest-- = '\0';
            if ( name[f_pos] == sep[0] ) f_pos += s_len;
            while ( name[f_pos] == ' ' ) f_pos++;

            dest = NULL;
        } else {
            if ( name[f_pos] == scheme[s_pos] ) f_pos++;
        }
    }

    fix_percentage_sequences ( Title   );
    fix_percentage_sequences ( Artist  );
    fix_percentage_sequences ( Album   );
    fix_percentage_sequences ( Track   );
    fix_percentage_sequences ( Genre   );
    fix_percentage_sequences ( Year    );
    fix_percentage_sequences ( Comment );

    if ( strlen ( Album ) > 6 && Year[0] == '\0' ) {
        int l = strlen ( Album );
        if ( (Album[l-6] == '(' || Album[l-6] == '[') && (Album[l-1] == ')' || Album[l-1] == ']') &&
            ((Album[l-5] == '1' && Album[l-4] == '9') || (Album[l-5] == '2' && Album[l-4] == '0')) &&
            (Album[l-3] >= '0' && Album[l-3] <= '9' && Album[l-2] >= '0' && Album[l-2] <= '9') ) {

            memcpy ( Year, (char *)(Album+l-5), 4 );
            Year[4] = '\0';
            if ( Album[l-7] == ' ' )
                Album[l-7] = '\0';
            else
                Album[l-6] = '\0';
        }
    }

    if ( Title[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_TITLE,   0, Title, 0, 0, guess );
    }
    if ( Artist[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_ARTIST,  0, Artist, 0, 0, guess );
    }
    if ( Album[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_ALBUM,   0, Album, 0, 0, guess );
    }
    if ( Track[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_TRACK,   0, Track, 0, 0, guess );
    }
    if ( Genre[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_GENRE,   0, Genre, 0, 0, guess );
    }
    if ( Year[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_YEAR,    0, Year, 0, 0, guess );
    }
    if ( Comment[0] != '\0' ) {
        InsertTagFieldLonger ( SIMPLETAG_FIELD_COMMENT, 0, Comment, 0, 0, guess );
    }

    tagtype = guessed_tag;

    return 0;
}

// gets pattern of filename scheme
int GuessTagFromName_Test ( const char* filename, const char* naming_scheme, char* pattern )
{
    char scheme  [_MAX_PATH];

    char name    [_MAX_PATH];
    char dir     [_MAX_DIR];
    char fname   [_MAX_FNAME];

    char *dest = NULL;      // destination
    char *sep  = NULL;      // field separator
    unsigned int s_pos = 0; // position in scheme
    unsigned int f_pos = 0; // position in filename
    unsigned int d_len = 0; // length of destination
    unsigned int s_len = 0; // length of source
    unsigned int pat_p = 0; // position in pattern
    unsigned long len;      // length of name
    int paths = 0;
    unsigned int i;

    strcpy ( name, filename );
    for ( i = 0; i < strlen (name) - 5; i++ ) {
#ifdef _WIN32
		if( name[i] == '\\')
		{
			name[i] = '/';
		}
#endif
        if ( name[i] != '/' ) continue;
        if ( strncasecmp ( (char *)(name+i+1), "(CD", 3 ) == 0 || strncasecmp ( (char *)(name+i+1), "(DVD", 4 ) == 0 )
            name[i] = ' ';
    }

    splitpath( name, dir, fname ,NULL);

    if ( strlen ( fname ) == 0 || strlen (naming_scheme) >= _MAX_PATH )
        return 1;

    for ( i = 0; i <= strlen (naming_scheme); i++ ) {
        char c = naming_scheme[i];
        if ( c == '/' ) {
            if ( scheme[i + 1] == '/' ) i++;
            paths++;
        }
        scheme[i] = c;
    }

    if ( paths > 0 ) {
        int path_pos = strlen ( dir ) - 1;

        while ( paths > 0 && path_pos >= 0 ) {
            if ( dir[--path_pos] == '/' ) paths--;
        }
        path_pos++;

        strcpy ( name, (char *)(dir+path_pos) );
        strcat ( name, fname );
        len = strlen ( name );
    } else {
        strcpy ( name, fname );
        len = strlen ( name );
    }

    *pattern = '\0';
    while ( scheme[s_pos] != '%' ) {
        pattern[pat_p++] = name[f_pos++];
        pattern[pat_p  ] = '\0';
        s_pos++;
    }

    for ( ; s_pos < strlen (scheme) + 1; s_pos++ ) {
        if ( scheme[s_pos] == '%' ) {
            s_pos++;
            sep = (char *)(scheme+s_pos+1);
            s_len = 0;
            while ( (sep < (char *)(scheme+strlen(scheme)+1)) && ((*(sep+s_len) != '%' && *(sep+s_len) != '\0')) ) s_len++;
            if ( s_len == 0 ) s_len = 1;

            switch ( scheme[s_pos] ) {
            case 't':   // title
            case 'T':
            case 'a':   // artist
            case 'A':
            case 'l':   // album
            case 'L':
            case 'c':   // comment
            case 'C':
            case 'n':   // track
            case 'N':
            case 'y':   // year
            case 'Y':
            case 'g':   // genre
            case 'G':
            case 'x':   // disrecard
            case 'X':
                dest  = (char *)(pattern + pat_p);
                d_len = 1;
                break;
            default:
                dest  = NULL;
                d_len = 0;
                break;
            }
        }

        if ( dest != NULL && sep != NULL ) {
            char *end = (char *)(dest + d_len);

            while ( f_pos < len && strncmp ((char*)(name+f_pos), sep, s_len) != 0 ) {
                if ( dest < end ) {
                    char c = name[f_pos++];
                    if ( c == '_' ) c = ' ';
                    if ( c == '\'' && f_pos < len && name[f_pos] == '\'' ) {    // '' -> "
                        f_pos++;
                        c = '"';
                    }
                    if ( c == ' ' && f_pos < len && name[f_pos] == ' ' ) {      // '  ' -> ' '
                        f_pos++;
                    }
                    *dest++ = c;
                } else {
                    f_pos++;
                }
            }

            *dest++ = '\0';
            if ( name[f_pos] == sep[0] ) f_pos += s_len;
            while ( name[f_pos] == ' ' ) f_pos++;
            strncat ( pattern, sep, s_len );
            pat_p = strlen ( pattern );
            dest = NULL;
        }
    }

    return 0;
}

// finds best matching naming scheme
int FindBestGuess ( const char* filename )
{
    static const char  identifiers [] = "ATLCGNYX";

    char               scheme  [_MAX_PATH];
    char               test    [_MAX_PATH];
    int                closest_match = -1;
    unsigned int       i, j, k;

    for ( i = 0; i < sizeof (SchemeList)/sizeof (*SchemeList); i++ ) {
        GuessTagFromName_Test ( filename, SchemeList[i], test );

        k = 0;
        for ( j = 0; j <= strlen ( SchemeList[i]); j++ ) {
            if ( SchemeList[i][j] == '%' ) continue;
            scheme[k++] = SchemeList[i][j];
        }

        if ( strlen ( test ) == strlen ( scheme ) ) {
            int match = 1;

            for ( j = 0; j < strlen ( test ); j++ ) {
                int is_identifier = 0;
                int tc = test[j];
                int sc = scheme[j];

                if ( sc >= 'a' && sc <= 'z' ) sc -= 'a'-'A';
                for ( k = 0; k < sizeof (identifiers); k++ ) {
                    if ( sc == identifiers[k] ) {
                        is_identifier = 1;
                        break;
                    }
                }

                if ( !is_identifier && sc == tc )   // field separator
                    continue;

                switch (sc) {
                case '/':   // path separator
                case '\\':
                    sc = '/';
                    break;
                case 'N':   // numerical
                case 'Y':
                    sc = 'N';
                    break;
                default:    // text
                    sc = 'A';
                    break;
                }

                if ( tc == '/' || tc == '\\' ) {
                    tc = '/';
                } else
                if ( tc >= '0' && tc <= '9' ) {
                    tc = 'N';
                } else {
                    tc = 'A';
                }

                if ( closest_match == -1 && (sc == 'A' && tc == 'N') )
                    closest_match = (int)i;

                if ( sc != tc ) {
                    if ( sc == 'N' && tc == 'A' && closest_match == (int)i ) {
                        closest_match = -1;
                    }
                    match = 0;
                }
            }

            if ( match ) return i;
        }
    }

    return closest_match;
}

// guess tag information from filename
int CSimpleTagReader::GuessTag ( const char* filename )
{
    int best;

    if ( (best = FindBestGuess ( filename )) < 0 )
        return 1;

    return GuessTagFromName ( filename, SchemeList[best] );
}

// searches and reads a ID3v1 tag
int CSimpleTagReader::ReadID3v1Tag (  CSimpleTagReader::CFile & fp,long TagOffset)
{
/*    unsigned */char   tmp [128];
/*    unsigned */char   value [32];
    char            utf8[32*2];

    if ( fp.seek ( TagOffset - sizeof (tmp), SEEK_SET ) != 0 )
        return 0;
    if ( fp.read  ( tmp, sizeof (tmp) ) != sizeof (tmp) )
        return 0;
    // check for id3-tag
    if ( 0 != memcmp (tmp, "TAG", 3) )
        return 0;

    memcpy_crop ( value, tmp +  3, 30 );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_TITLE  , 0, utf8, 0, 0,ID3v1 );
    }
    memcpy_crop ( value, tmp + 33, 30 );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_ARTIST , 0, utf8, 0, 0,ID3v1 );
    }
    memcpy_crop ( value, tmp + 63, 30 );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_ALBUM  , 0, utf8, 0, 0,ID3v1 );
    }
    memcpy_crop ( value, tmp + 93,  4 );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_YEAR   , 0, utf8, 0, 0,ID3v1 );
    }
    memcpy_crop ( value, tmp + 97, 30 );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_COMMENT, 0, utf8, 0, 0,ID3v1 );
    }

    if ( tmp[125] == 0 ) {
        sprintf ( value, "%d", tmp[126] );
        if ( value[0] != '\0' && atoi (value) != 0 ) {
            ConvertANSIToUTF8 ( value, utf8 );
            InsertTagFieldLonger ( SIMPLETAG_FIELD_TRACK, 0, utf8, 0, 0,ID3v1 );
        }
    }
    GenreToString ( value, tmp[127] );
    if ( value[0] != '\0' ) {
        ConvertANSIToUTF8 ( value, utf8 );
        InsertTagFieldLonger ( SIMPLETAG_FIELD_GENRE  , 0, utf8, 0, 0,ID3v1 );
    }

    TagOffset -= 128;
    tagtype = ID3v1_tag;

    return 0;
}


struct APETagFooterStruct {
    unsigned char   ID       [8];    // should equal 'APETAGEX'
    unsigned char   Version  [4];    // 1000 = version 1.0, 2000 = version 2.0
    unsigned char   Length   [4];    // complete size of the tag, including footer, excluding header
    unsigned char   TagCount [4];    // number of fields in the tag
    unsigned char   Flags    [4];    // tag flags (none currently defined)
    unsigned char   Reserved [8];    // reserved for later use
};


static unsigned long
Read_LE_Uint32 ( const unsigned char* p )
{
    return ((unsigned long)p[0] <<  0) |
           ((unsigned long)p[1] <<  8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}
/*
static void
Write_LE_Uint32 ( unsigned char* p, const unsigned long value )
{
    p[0] = (unsigned char) (value >>  0);
    p[1] = (unsigned char) (value >>  8);
    p[2] = (unsigned char) (value >> 16);
    p[3] = (unsigned char) (value >> 24);
}
*/
int
CSimpleTagReader::ReadAPE1Tag ( CSimpleTagReader::CFile & fp,long TagOffset)
{
    unsigned long               vsize;
    unsigned long               isize;
    unsigned long               flags;
    /*unsigned */char*              buff;
    /*unsigned */char*              p;
    /*unsigned */char*              end;
    char*                       utf8;
    struct APETagFooterStruct   T;
    unsigned long               TagLen;
    unsigned long               TagCount;

    if ( fp.seek ( TagOffset - sizeof T, SEEK_SET ) != 0 )
        return 0;
    if ( fp.read ( &T, sizeof T ) != sizeof T )
        return 0;
    if ( memcmp ( T.ID, "APETAGEX", sizeof T.ID ) != 0 )
        return 0;
    if ( Read_LE_Uint32 (T.Version) != 1000 )
        return 0;
    TagLen = Read_LE_Uint32 (T.Length);
    if ( TagLen <= sizeof T )
        return 0;
    if ( fp.seek ( TagOffset - TagLen, SEEK_SET ) != 0 )
        return 0;
    buff = (char *)malloc ( TagLen );
    if ( buff == NULL )
        return 1;
    if ( fp.read ( buff, TagLen - sizeof T ) != TagLen - sizeof T ) {
        free ( buff );
        return 0;
    }

    TagCount = Read_LE_Uint32 (T.TagCount);
    end = buff + TagLen - sizeof (T);
    for ( p = buff; p < end /*&& *p */ &&  TagCount--; ) {
        vsize = Read_LE_Uint32 ( (unsigned char *) p ); p += 4;
        flags = Read_LE_Uint32 ( (unsigned char *) p ); p += 4;
        isize = strlen (p);

        if ( vsize > 0 ) {
            if ( (utf8 = (char*)malloc ( (vsize + 1) * 3 )) == NULL ) {
                free ( buff );
                return 1;
            }
            ConvertANSIToUTF8 ( p + isize + 1, utf8 );
            InsertTagFieldLonger ( p, isize, utf8, 0, 0,APE1 );  // flags not used with APE 1.0
            free ( utf8 );
        }
        p += isize + 1 + vsize;
    }

    TagOffset -= TagLen;
    tagtype = APE1_tag;

    free ( buff );

    return 0;
}


int
CSimpleTagReader::ReadAPE2Tag ( CSimpleTagReader::CFile & fp ,long TagOffset)
{
    unsigned long               vsize;
    unsigned long               isize;
    unsigned long               flags;
    /*unsigned */char*              buff;
    /*unsigned */char*              p;
    /*unsigned */char*              end;
    struct APETagFooterStruct   T;
    unsigned long               TagLen;
    unsigned long               TagCount;
    size_t                      i;

    if ( fp.seek ( TagOffset - sizeof T, SEEK_SET ) != 0 )
        return 0;
    if ( fp.read ( &T, sizeof T ) != sizeof T )
        return 0;
    if ( memcmp ( T.ID, "APETAGEX", sizeof T.ID ) != 0 )
        return 0;
    if ( Read_LE_Uint32 (T.Version) != 2000 )
        return 0;
    TagLen = Read_LE_Uint32 (T.Length);
    if ( TagLen <= sizeof T )
        return 0;
    if ( fp.seek ( TagOffset - TagLen, SEEK_SET ) != 0 )
        return 0;
    buff = (char *)malloc ( TagLen );
    if ( buff == NULL )
        return 1;
    if ( fp.read ( buff, TagLen - sizeof T ) != TagLen - sizeof T ) {
        free ( buff );
        return 0;
    }

    TagCount = Read_LE_Uint32 (T.TagCount);
    end = buff + TagLen - sizeof (T);
    for ( p = buff; p < end /*&& *p */ &&  TagCount--; ) {
        vsize = Read_LE_Uint32 ( (unsigned char*)p ); p += 4;
        flags = Read_LE_Uint32 ( (unsigned char*)p ); p += 4;
        isize = strlen (p);

        if ( vsize > 0 )
            InsertTagFieldLonger ( p, isize, p + isize + 1, vsize, flags,APE2 );
        p += isize + 1 + vsize;
    }

    TagOffset -= TagLen;
    tagtype = APE2_tag;

    free ( buff );

    for ( i = 0; i < tagitem_count; i++ ) {
        if ( ReplaceListSeparator ( "\0", ListSeparator,i ) != 0 )
            return 2;
    }

    if ( Read_LE_Uint32 (T.Flags) & 1<<31 ) {       // Tag contains header
        TagOffset -= sizeof (T);
    } else {                                        // Check if footer was incorrect
        if ( fp.seek ( TagOffset - sizeof T, SEEK_SET ) != 0 )
            return 0;
        if ( fp.read ( &T, sizeof T ) != sizeof T )
            return 0;
        if ( memcmp ( T.ID, "APETAGEX", sizeof T.ID ) != 0 )
            return 0;
        if ( Read_LE_Uint32 (T.Version) != 2000 )
            return 0;
        if ( Read_LE_Uint32 (T.Flags) & 1<<29 )     // This is header
            TagOffset -= sizeof T;
    }

    return 0;
}



// scan file for all supported tags and read them
int CSimpleTagReader::ReadTags ( CSimpleTagReader::CFile & fp ,const char * filename)
{
    long TagOffs;
	if(fp.seek ( 0L, SEEK_END ) != 0 )// get filelength
		return 1;
	long TagOffset = fp.tell ();

    do {
        TagOffs = TagOffset;
        ReadAPE1Tag  ( fp , TagOffs);
        ReadAPE2Tag  ( fp , TagOffs);
        ReadID3v1Tag ( fp , TagOffs);
    } while ( TagOffs != TagOffset );

    if ( tagtype == no_tag && filename) 
	{
        return GuessTag ( filename );
    }

    return 0;
}
void  CSimpleTagReader::FreeTagFields ()
{
	size_t  i;

	if ( tagitems != NULL ) {
		for ( i = 0; i < tagitem_count; i++ ) {
			if ( tagitems[i].Item  ) free ( tagitems[i].Item  );
			if ( tagitems[i].Value ) free ( tagitems[i].Value );
		}

		free ( tagitems );
	}

	tagitem_count = 0;
	tagtype       = (tag_t)0;
	tagitems      = NULL;
}
CSimpleTagReader::CSimpleTagReader()
{
	tagitem_count = 0;
	tagtype       = (tag_t)0;
	tagitems      = NULL;
}
CSimpleTagReader::~CSimpleTagReader()
{
	FreeTagFields();
}

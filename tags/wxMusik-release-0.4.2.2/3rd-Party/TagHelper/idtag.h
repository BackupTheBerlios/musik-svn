/* This code was copied from winamp mpc plugin v.98                 */
/* http://www.saunalahti.fi/~cse/mpc/winamp/						*/
/* copyright Andree Buschmann, Andree.Buschmann@web.de              */
/* license of the code is LPGL						                */
/* modified by gunnar roth (gunnar.roth@gmx.de) Nov 10. 2004		*/


#ifndef _idtag_h_
#define _idtag_h_

/* D E F I N E S */

#define NO_GENRES      148

// Tag item names derived from tag fields
#define SIMPLETAG_FIELD_TITLE             "Title"
#define SIMPLETAG_FIELD_SUBTITLE          "Subtitle"
#define SIMPLETAG_FIELD_ARTIST            "Artist"
#define SIMPLETAG_FIELD_ALBUM             "Album"
#define SIMPLETAG_FIELD_DEBUTALBUM        "Debut Album"
#define SIMPLETAG_FIELD_PUBLISHER         "Publisher"
#define SIMPLETAG_FIELD_CONDUCTOR         "Conductor"
#define SIMPLETAG_FIELD_COMPOSER          "Composer"
#define SIMPLETAG_FIELD_COMMENT           "Comment"
#define SIMPLETAG_FIELD_YEAR              "Year"
#define SIMPLETAG_FIELD_RECORDDATE        "Record Date"
#define SIMPLETAG_FIELD_RECORDLOCATION    "Record Location"
#define SIMPLETAG_FIELD_TRACK             "Track"
#define SIMPLETAG_FIELD_GENRE             "Genre"
#define SIMPLETAG_FIELD_COVER_ART_FRONT   "Cover Art (front)"
#define SIMPLETAG_FIELD_NOTES             "Notes"
#define SIMPLETAG_FIELD_LYRICS            "Lyrics"
#define SIMPLETAG_FIELD_COPYRIGHT         "Copyright"
#define SIMPLETAG_FIELD_PUBLICATIONRIGHT  "Publicationright"
#define SIMPLETAG_FIELD_FILE              "File"
#define SIMPLETAG_FIELD_MEDIA             "Media"
#define SIMPLETAG_FIELD_EANUPC            "EAN/UPC"
#define SIMPLETAG_FIELD_ISRC              "ISRC"
#define SIMPLETAG_FIELD_RELATED_URL       "Related"
#define SIMPLETAG_FIELD_ABSTRACT_URL      "Abstract"
#define SIMPLETAG_FIELD_BIBLIOGRAPHY_URL  "Bibliography"
#define SIMPLETAG_FIELD_BUY_URL           "Buy URL"
#define SIMPLETAG_FIELD_ARTIST_URL        "Artist URL"
#define SIMPLETAG_FIELD_PUBLISHER_URL     "Publisher URL"
#define SIMPLETAG_FIELD_FILE_URL          "File URL"
#define SIMPLETAG_FIELD_COPYRIGHT_URL     "Copyright URL"
#define SIMPLETAG_FIELD_INDEX             "Index"
#define SIMPLETAG_FIELD_INTROPLAY         "Introplay"
#define SIMPLETAG_FIELD_MJ_METADATA       "Media Jukebox Metadata"
#define SIMPLETAG_FIELD_DUMMY             "Dummy"

enum tag_t {
    auto_tag    =  -1,
    no_tag      =   0,
    ID3v1_tag   =   1,
    APE1_tag    =   2,
    APE2_tag    =   3,
    Ogg_tag     =   4,
    ID3v2       =  16,
    guessed_tag = 255,
};

// reliability of tag information
enum tag_rel {
    guess       =  1,
    ID3v1       = 10,
    APE1        = 19,
    APE2        = 20
};

/* V A R I A B L E S */
extern const char* const GenreList [];   // holds the genres available for ID3

/* F U N C T I O N S */



class CSimpleTagReader
{
private:
	typedef struct {
		/*unsigned */char*      Item;               // Name of item     (ASCII)
		/*unsigned */char*      Value;              // Value of item    (UTF-8)
		unsigned int        Flags;              // Flags
		size_t              ItemSize;           // Length of name   (bytes)
		size_t              ValueSize;          // Length of value  (bytes)
		enum tag_rel        Reliability;        // Reliability of information
	} TagItem;

	size_t              tagitem_count;
	TagItem*            tagitems;
	enum tag_t          tagtype;
public:
	class CFile
	{
	private:
		FILE *f;
		bool m_bOwner;
	public:
		CFile(FILE * _f,bool bOwner = true) {f=_f;m_bOwner=bOwner;}
		virtual ~CFile() {if (m_bOwner && f) fclose(f);}
		virtual size_t read(void * ptr,size_t size) {return fread(ptr,1,size,f);}
		virtual int seek(int offset,int origin) {return fseek(f,offset,origin);}
		virtual long tell() {return ftell(f);}
	};

	CSimpleTagReader();
	~CSimpleTagReader();
	// Read tag data
	int   ReadID3v1Tag  ( CSimpleTagReader::CFile & fp, long TagOffset = -1);              // reads ID3v1 tag
	int   ReadAPE1Tag   ( CSimpleTagReader::CFile & fp,long TagOffset = -1);                         // reads APE 1.0 tag
	int   ReadAPE2Tag   ( CSimpleTagReader::CFile & fp,long TagOffset = -1);                         // reads APE 2.0 tag
	int   ReadTags      ( CSimpleTagReader::CFile & fp,const char * filename = NULL);                // reads all tags from file
	// Function that tries to guess tag information from filename
	int GuessTag ( const char* filename);

	// Free all tag fields from memory
	void  FreeTagFields ( );
	// Copy item to dest, maximum length to copy is count
	size_t   CopyTagValue  ( char* dest, const char* item, size_t count );
	// Return pointer to item or NULL on error
	char* TagValue      ( const char* item);

	static int  GenreToInteger ( const char* GenreStr);
protected:
	int NewTagField ( const char* item, const size_t itemsize, const char* value, const size_t valuesize, const unsigned int flags,  const enum tag_rel reliability );
	int ReplaceTagField ( const char* value, const size_t valuesize, const unsigned int flags, size_t itemnum, const enum tag_rel reliability );
	int InsertTagField ( const char* item, size_t itemsize, const char* value, size_t valuesize, const unsigned int flags);                     
	int InsertTagFieldLonger ( const char* item, size_t itemsize, const char* value, size_t valuesize, const unsigned int flags, const enum tag_rel reliability );
	int ReplaceListSeparator ( const char* old_sep, const char* new_sep, size_t itemnum );
	int ConvertTagFieldToUTF8 ( size_t itemnum );
	int GuessTagFromName ( const char* filename, const char* naming_scheme);

};





#endif

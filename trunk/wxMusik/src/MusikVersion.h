#ifndef MUSIK_VERSION_H
#define MUSIK_VERSION_H
#define MUSIK_OFFICIAL_BUILD 1

#define MUSIK_STRINGIZE2(x)  #x
#define MUSIK_STRINGIZE_T(x)  wxT(MUSIK_STRINGIZE2(x))
#define MUSIK_STRINGIZE(x)		MUSIK_STRINGIZE2(x)

#define MUSIK_VERSION_MAJOR		0
#define MUSIK_VERSION_MAJORSUB	4
#define MUSIK_VERSION_MINOR		1
#define MUSIK_VERSION_MINORSUB	0

#define MUSIK_VERSION(ma,mas,mi,mis) ((((unsigned long)((char)ma))<<24) | (((unsigned long)((char)mas))<<16) | (((unsigned long)((char)mi))<<8) | (((unsigned long)((char)mis))))
#define MUSIK_VERSION_CURRENT MUSIK_VERSION(MUSIK_VERSION_MAJOR,MUSIK_VERSION_MAJORSUB,MUSIK_VERSION_MINOR,MUSIK_VERSION_MINORSUB) 
#ifdef __WXDEBUG__
#define MUSIK_VERSION_DEBUG wxT("-DEBUG-")
#else
#define MUSIK_VERSION_DEBUG wxT("")
#endif
#ifdef MUSIK_OFFICIAL_BUILD
#define MUSIK_VERSION_ADDENDUM	wxT("")
#else
#ifdef RC_INVOKED
#define MUSIK_VERSION_ADDENDUM	wxT(" CVS ")// wxT(__DATE__) 
#else
#define MUSIK_VERSION_ADDENDUM	wxT(" CVS ") wxT(__DATE__) 
#endif   // #else RC_INVOKED
#endif	 // #else MUSIK_OFFICIAL_BUILD

#define MUSIK_VERSION_STR		MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJOR) wxT(".") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJORSUB) wxT(".") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MINOR) wxT(".") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MINORSUB) 
#define MUSIK_VERSION_COMMA_STR		MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJOR) wxT(",") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJORSUB) wxT(",") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MINOR) wxT(",") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MINORSUB) 

#ifdef MUSIK_OFFICIAL_BUILD
#define MUSIK_TITLE_VERSION_STR	MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJOR) wxT(".") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MAJORSUB) wxT(".") \
	MUSIK_STRINGIZE_T(MUSIK_VERSION_MINOR) 
#else
#define MUSIK_TITLE_VERSION_STR	MUSIK_VERSION_STR\
	MUSIK_VERSION_DEBUG \
	MUSIK_VERSION_ADDENDUM
#endif   //else MUSIK_OFFICIAL_BUILD
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include <wx/wxprec.h>

#ifdef WX_PRECOMP
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/tokenzr.h>
#include <wx/dir.h>
#include <wx/mimetype.h>
#if wxUSE_STATLINE
#include "wx/statline.h"
#endif
#include <wx/regex.h>
#include <wx/listctrl.h>
#include <wx/textdlg.h>
#include <wx/dynarray.h>
#include <wx/app.h>
#include <wx/dcscreen.h>
#include <wx/textctrl.h>
#include <wx/dcmemory.h>
#include <wx/dcbuffer.h>
#include <wx/selstore.h>
#include <wx/renderer.h>
#include <wx/math.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/gauge.h>
#include <wx/laywin.h>
#if wxUSE_MIMETYPE
#ifdef __WXMSW__
#include <shlobj.h>
#endif
#include <wx/mimetype.h>
#endif
#ifdef wxHAS_TASK_BAR_ICON
#include "wx/taskbar.h"
#ifdef __WXMSW__
#include <ShellAPI.h>
#endif
#endif
#if wxCHECK_VERSION(2,5,4)
#ifdef __WXMSW__
#include <wx/stdpaths.h>
#endif
#endif
#include <wx/valgen.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>

#include <wx/socket.h>
#ifdef wxHAS_TASK_BAR_ICON
#include "wx/taskbar.h"
#endif
#include "wx/thread.h"
#include "wx/laywin.h"

#include "wx/dnd.h"
#include "wx/dcbuffer.h"
#include <wx/dynarray.h>
#include <wx/treectrl.h>
#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h> 
#include <wx/filedlg.h>
#include <wx/colordlg.h>
#include <wx/spinctrl.h>
#include <wx/textfile.h>
#include <wx/checklst.h>
#include <wx/valgen.h>
#include <wx/cmdline.h>
#include <wx/progdlg.h>
#include <wx/sysopt.h>
#include <wx/hashmap.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/listctrl.h>
#include <wx/textdlg.h>
#include <wx/spinbutt.h>
#include <wx/listbase.h>
#include <wx/intl.h>
#include <wx/fileconf.h>
#include <wx/url.h>
#include <wx/txtstrm.h>

#include <map>
#include <vector>
#include <exception>

#include <taglib.h>

#include <fileref.h>
#include <tag.h>
#include <tfile.h>
#include <mpeg/mpegproperties.h>
#include <mpeg/mpegfile.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <mpeg/id3v2/frames/attachedpictureframe.h>
#ifdef USE_FMOD3
#include <fmod/fmod.h>
#endif
#ifdef USE_FMODEX
#include <fmodex/fmod.hpp>
#endif#
#include <FLAC/format.h>
#include <FLAC/metadata.h>

#endif

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

#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h> 

#include <map>

#endif
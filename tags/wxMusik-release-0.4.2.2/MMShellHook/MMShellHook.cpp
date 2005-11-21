/*
 *  MMShellHook.cpp
 *
 *  MMShellHook is a quick little DLL that is used
 *	to handle Windows Multimedia Keyboards
 *  
 *  Copyright (c) 2003 Gunnar Roth
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include "MMShellHook.h"
#include <tchar.h>
#include <windows.h>
#include <winuser.h>
#include <crtdbg.h>

//-------------------------------------------------//
//--- Storage for the global data in the DLL	---//
//-------------------------------------------------//
#pragma data_seg(".shared") // create a section named .shared. 
						    //#pragma section(".shared",read,write,shared), 
							// makes this section shared between all instances of this dll in all processes
HWND	hNotifyWnd = NULL;
HHOOK	hShellHook = NULL;	//--- Handle to the Shell hook	---//
#pragma data_seg( )

#pragma section(".shared",read,write,shared)

//-------------------------------------------------//
//--- Per-instance DLL variables				---//
//-------------------------------------------------//
HINSTANCE hInstance = NULL;		//--- This instance of the DLL	---//
LRESULT CALLBACK ShellProc (int nCode, WPARAM wParam, LPARAM lParam);

//-------------------------------------------------//
//--- The DLL's main procedure					---//
//-------------------------------------------------//
BOOL WINAPI DllMain ( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
	//-----------------------------------------//
	//--- Find out why we're being called	---//
	//-----------------------------------------//
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		_RPT0( _CRT_WARN, "MMShellHook: Hook DLL loaded\n" );
		hInstance = (HINSTANCE)hInst;		//--- Save the instance handle										---//
		return TRUE;						//--- ALWAYS return TRUE to avoid breaking unhookable applications	---//

	case DLL_PROCESS_DETACH:
		_RPT0( _CRT_WARN, "MMShellHook: Hook DLL unloaded\n" );
		return TRUE;

	default:
		return TRUE;
	}
}

DllExport BOOL SetMMShellHook( HWND hWnd )
{
	//-----------------------------------------------------//
	//--- Don't add the hook if the window ID is NULL	---//
	//--- or if a hook already exists.					---//
	//-----------------------------------------------------//
	if ( hWnd == NULL || hNotifyWnd != NULL )
		return FALSE;
	
	//-----------------------------------------------------//
	//--- Add the ShellProc hook						---//
	//-----------------------------------------------------//
	hShellHook = SetWindowsHookEx
				(
					WH_SHELL,						//--- Hook in before msg reaches app	---//
					(HOOKPROC) ShellProc,			//--- Hook procedure					---//
					hInstance,						//--- This DLL instance					---//
					0L								//--- Hook in to all apps				---//
				);							


	//-----------------------------------------------------//
	//--- Make sure the hook was was set correctly		---//
	//-----------------------------------------------------//
	if (hShellHook != NULL)
	{
		hNotifyWnd = hWnd;							//--- Save the WinRFB window handle		---//
		_RPT1(_CRT_WARN, "Notify Window is: %d\n", hWnd);
		return TRUE;
	}

	//-----------------------------------------------------//
	//--- If we got here the hook failed, return false	---//
	//-----------------------------------------------------//
	return FALSE;
}


DllExport BOOL UnSetMMShellHook(HWND hWnd)
{
	BOOL unHooked = TRUE;
	
	//-----------------------------------------------------//
	//--- If there is an invalid window handle or the	---//
	//--- wrong app is calling UnSetHook return false	---//
	//-----------------------------------------------------//
	if ( hWnd == NULL || hWnd != hNotifyWnd )
		return FALSE;

	//-----------------------------------------------------//
	//--- unhook the process							---//
	//-----------------------------------------------------//
	if ( hNotifyWnd != NULL )
	{
		unHooked = UnhookWindowsHookEx( hShellHook );
		hShellHook = NULL;
	}

	//-----------------------------------------------------//
	//--- if unhook was succeeded, reset the handle		---//
	//-----------------------------------------------------//
	if (unHooked)
		hNotifyWnd = NULL;

	return unHooked;
}



LRESULT CALLBACK ShellProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	//-----------------------------------------------------//
	//--- this is the hook callback. first see if we	---//
	//--- need to handle this message.					---//
	//-----------------------------------------------------//
	if (nCode == HSHELL_APPCOMMAND)
	{
		//-----------------------------------------------------//
		//--- make sure there is a valid window handle		---//
		//-----------------------------------------------------//
		if ( hNotifyWnd != NULL )
		{
			short AppCommand = GET_APPCOMMAND_LPARAM( lParam );	
			switch (AppCommand)
			{
			case APPCOMMAND_MEDIA_NEXTTRACK:
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			case APPCOMMAND_MEDIA_STOP:
				::PostMessage(hNotifyWnd,WM_APPCOMMAND,wParam,lParam);
				return 1; // dont call CallNextHookEx, instead return non-zero, because we have handled the message (see MSDN doc)
			}
		}
	}

	//---------------------------------------------------------//
	//--- event wasn't for uss? call the next handler in	---//
	//--- the chain.										---//
	//---------------------------------------------------------//
    return CallNextHookEx ( hShellHook, nCode, wParam, lParam );
}

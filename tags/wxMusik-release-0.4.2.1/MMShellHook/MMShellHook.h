/*
 *  MMShellHook.h
 *
 *  MMShellHook is a quick little DLL that is used
 *	to handle Windows Multimedia Keyboards
 *  
 *  Copyright (c) 2003 Gunnar Roth
 *
 *  See the file "license.txt" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

#if !defined(_MMShellHook_DLL_)
#define _MMShellHook_DLL_

#include <windows.h>

//-------------------------------------//
//--- Define the import/export tags ---//
//-------------------------------------//
#define DllImport __declspec( dllimport )
#define DllExport __declspec( dllexport )

//-------------------------------------//
//--- Functions used				---//
//-------------------------------------//

extern "C"
{
	DllExport BOOL SetMMShellHook	( HWND hWnd	);	
	DllExport BOOL UnSetMMShellHook	( HWND hWnd );	
}

#endif 

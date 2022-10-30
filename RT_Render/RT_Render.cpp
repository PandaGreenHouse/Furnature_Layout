// RT_Render.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "RT_Render.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CRT_RenderApp

BEGIN_MESSAGE_MAP(CRT_RenderApp, CWinApp)
END_MESSAGE_MAP()


// CRT_RenderApp construction

CRT_RenderApp::CRT_RenderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CRT_RenderApp object

CRT_RenderApp theApp;


// CRT_RenderApp initialization

BOOL CRT_RenderApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}



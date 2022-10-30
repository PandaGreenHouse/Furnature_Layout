// RT_Render.h : main header file for the RT_Render DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRT_RenderApp
// See RT_Render.cpp for the implementation of this class
//

class CRT_RenderApp : public CWinApp
{
public:
	CRT_RenderApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

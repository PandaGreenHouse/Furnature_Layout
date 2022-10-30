#pragma once

#ifndef __RENDER_MODEL_STRUCTURES_HEADER__
#define __RENDER_MODEL_STRUCTURES_HEADER__

#include <tchar.h>
#include "MyArray.h"
#include "stdafx.h"
#include "MainProcess.h"
#include "ObjectNode.h"


extern CMyArray<XML_MATINFO>	g_arrMats;
extern CMyArray<ObjectNode*>	g_arrObjects;

extern CMainProcess g_MainProcess;


extern void WndRedraw();


extern LPXML_MATINFO GetMaterialObject(int nObject);
extern TCHAR* GetPutModeString(int nIndex);
extern TCHAR* GetObjectXmlTagName(int nType);
extern bool CheckFileExists(LPTSTR lpszPath);


#endif	//__RENDER_MODEL_STRUCTURES_HEADER__
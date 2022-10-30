#pragma once
#include "MyStruct.h"

#ifndef __RENDER_VIEW_GLOBALS_HEADER__
#define __RENDER_VIEW_GLOBALS_HEADER__

#ifdef RENDERVIEW_EXPORTS
#define RENDERVIEW_API		_declspec(dllexport)
#else
#define RENDERVIEW_API		_declspec(dllimport)
#endif

#define RENDERVIEW_CC		__stdcall

extern "C"
{
	RENDERVIEW_API HWND RENDERVIEW_CC CreateRenderWindow(HINSTANCE hInstance, HWND hWndParent, int nWidth, int nHeight, int nLeft, int nTop);
	RENDERVIEW_API void RENDERVIEW_CC DestroyRenderWindow();

	RENDERVIEW_API void RENDERVIEW_CC ShowHideRenderView(int bShow);

	RENDERVIEW_API void RENDERVIEW_CC SetViewMode(int nMode);

	RENDERVIEW_API int  RENDERVIEW_CC LoadModel(LPCTSTR lpszFileName, LPCTSTR lpszContent);
	RENDERVIEW_API int  RENDERVIEW_CC LoadModelByContent(LPCTSTR lpszContent);
	RENDERVIEW_API int  RENDERVIEW_CC SaveModel(LPCTSTR lpszFileName);

	RENDERVIEW_API int  RENDERVIEW_CC GetMaterialCount();
	RENDERVIEW_API int  RENDERVIEW_CC GetMaterialInfo(int nIndex, LPOUT_MATINFO pMat);

	RENDERVIEW_API int  RENDERVIEW_CC AddMaterial(LPOUT_MATINFO pMat);
	RENDERVIEW_API int  RENDERVIEW_CC EditMaterial(int nIndex, LPOUT_MATINFO pMat);
	RENDERVIEW_API int  RENDERVIEW_CC DeleteMaterial(int nIndex);

	RENDERVIEW_API int  RENDERVIEW_CC ApplyTexture(int nObject, int nMatIdx);

	RENDERVIEW_API void RENDERVIEW_CC SetLineDrawFlag(int nFlag);
	RENDERVIEW_API void RENDERVIEW_CC RotateObject(int nDegree);	//nDegree: -360 ~ 360	Positive Value Rotates in CW(Clockwise) Direction

	//Returns Selected Object (Index at the Objects Array) and fill its Data to a structure
	//Parameter: pObj - Structure Pointer to Fill Object Data
	//Return Value: Selected Object Index
	RENDERVIEW_API int  RENDERVIEW_CC GetSelectedObject(LPOUT_OBJECTINFO pObj);
	//Returns Selected Object (Index at the Objects Array) and fill its Data to a structure
	//Parameter: nObject - Object Index (at the Objects Array) to Get Parent Item
	//		pObj - Structure Pointer to Fill Parent Object Data
	//Return Value: Parent Object Index
	RENDERVIEW_API int  RENDERVIEW_CC GetParentObject(int nObject, LPOUT_OBJECTINFO pObj);

	//Edit Object Data
	//Parameter: nObject - Object Index (at the Objects Array) to Edit
	//			pObj - Structure Pointer to New Object Data
	//Return Value: Parent Object Index
	RENDERVIEW_API int  RENDERVIEW_CC EditSelectedObject(int nObject, LPOUT_OBJECTINFO pObj);

	//Get Children Item Info of Group
	//Parameter:
	//			ppNames - attribute name array
	//Return Value: Pointer to Info String
	RENDERVIEW_API TCHAR* RENDERVIEW_CC GetGroupNextInfo(TCHAR** ppNames, int nCount);

	//Free Result String Buffer
	RENDERVIEW_API void	 RENDERVIEW_CC ReleaseString(void* pString);

	RENDERVIEW_API void	 RENDERVIEW_CC ScatterObjects(int nCount);
	RENDERVIEW_API void RENDERVIEW_CC RT_Render();
	RENDERVIEW_API void RENDERVIEW_CC RayTracingRender(HDC hdc, int nShadingMode, int Nx, int Ny);
	
};

extern int GetImageAbsolutePath(LPTSTR lpszAbsPath, LPTSTR lpszFile);

#endif	//__RENDER_VIEW_GLOBALS_HEADER__
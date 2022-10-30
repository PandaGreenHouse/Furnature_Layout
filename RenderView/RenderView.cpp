// RenderView.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <tchar.h>
#include "RenderView.h"
#include "Model.h"
#include "MainProcess.h"
#include "RayRender.h"


CRayRender g_ClsRayRender;
TCHAR		g_szModeText[][20] = {_T("四格图"), _T("俯视图"), _T("正视图"), _T("侧视图"), _T("三维图"), _T("线框图"), _T("透视图")}; 
TCHAR		g_szWndClass[128] = _T("MyRenderView");	//Window Class Name

HWND		g_hWnd = NULL;		//Window Handle
HINSTANCE	g_hAppInst = NULL;	//Current Application (EXE) Instance
HINSTANCE	g_hDllInst = NULL;	//Current DLL(RenderView.dll) Instance

CMainProcess	g_MainProcess;	//Main Process Object


// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
void MyUnRegisterClass();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void LogText(LPTSTR lpszFileName, LPTSTR lpszText);

HWND RENDERVIEW_CC CreateRenderWindow(HINSTANCE hInstance, HWND hWndParent, int nWidth, int nHeight, int nLeft, int nTop)
{
	if (g_hWnd == NULL)
	{
		if (g_hAppInst != hInstance)
		{
			if (g_hAppInst)
				MyUnRegisterClass();
			g_hAppInst = hInstance;
			MyRegisterClass(hInstance);
		}
		g_hWnd = ::CreateWindow(g_szWndClass, NULL, WS_CHILD | WS_VISIBLE, nLeft, nTop, nWidth, nHeight, hWndParent, NULL, g_hAppInst, NULL);
	}
	return g_hWnd;
}

void RENDERVIEW_CC DestroyRenderWindow()
{
	if (g_hWnd)
	{
		g_MainProcess.SetHWnd(NULL);
		DestroyWindow(g_hWnd);
		g_hWnd = NULL;
	}
}

//Show or Hide Render View
//Parameters:
//		bShow: If NonZero - Show Render View, 0 - Hide Render View
void RENDERVIEW_CC ShowHideRenderView(int bShow)
{
	if (g_hWnd == NULL)
		return;

	::ShowWindow(g_hWnd, (bShow) ? SW_SHOW : SW_HIDE);
}


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD  dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hDllInst = hInstance;
		break;

	case DLL_PROCESS_DETACH:
		g_hDllInst = NULL;
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT	res = 0;
	POINT	CurPt;
	RECT	rcClient;
	HDC		hdc;
	
	if (g_hWnd == NULL || g_hWnd != hWnd)
		g_hWnd = hWnd;
	if (g_MainProcess.GetHWnd() == NULL)
		g_MainProcess.SetHWnd(hWnd);
	switch (message)
	{
	case WM_ERASEBKGND:
		if (g_arrObjects.GetCount() < 1)
		{
			hdc = GetDC(hWnd);
			if (hdc)
			{
				::GetClientRect(hWnd, &rcClient);
				FillRect(hdc, &rcClient, (HBRUSH) GetStockObject(WHITE_BRUSH));
				ReleaseDC(hWnd, hdc);
			}
		}
		
		g_MainProcess.RenderScene();
		break;

	case WM_LBUTTONDOWN:
		CurPt.x = LOWORD(lParam);
		CurPt.y = HIWORD(lParam);
		g_MainProcess.OnLButtonDown(&CurPt);
		break;

	case WM_LBUTTONUP:
		CurPt.x = LOWORD(lParam);
		CurPt.y = HIWORD(lParam);
		g_MainProcess.OnLButtonUp(&CurPt);
		break;

	case WM_RBUTTONDOWN:
		CurPt.x = LOWORD(lParam);
		CurPt.y = HIWORD(lParam);
		g_MainProcess.OnRButtonDown(&CurPt);
		break;

	case WM_MOUSEMOVE:
		CurPt.x = LOWORD(lParam);
		CurPt.y = HIWORD(lParam);
		SetFocus(hWnd);
		if (wParam & MK_LBUTTON)	//Move with LBUTTON Pressed
			g_MainProcess.OnMouseMove(&CurPt);
		break;

	case WM_MOUSEWHEEL:
		CurPt.x = ((int)(short)LOWORD(lParam));
		CurPt.y = ((int)(short)HIWORD(lParam));
		ScreenToClient(hWnd, &CurPt);
		if (HIWORD(wParam)> WHEEL_DELTA)
			g_MainProcess.ScrollDown(&CurPt);
		else if(HIWORD(wParam) == WHEEL_DELTA)
			g_MainProcess.ScrollUp(&CurPt);
		break;

	case WM_SIZE:
	case WM_WINDOWPOSCHANGED:
	case WM_WINDOWPOSCHANGING:
		g_MainProcess.ReSize();
		break;
	case WM_SETFOCUS:
	case WM_SHOWWINDOW:
		g_MainProcess.ReSize();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return res;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;	//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL; //MAKEINTRESOURCE(IDC_APITEST);
	wcex.lpszClassName	= g_szWndClass;
	wcex.hIcon			= NULL;	//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APITEST));
	wcex.hIconSm		= NULL; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

void MyUnRegisterClass()
{
	UnregisterClass(g_szWndClass, g_hAppInst);
	g_hAppInst = NULL;
}

void RENDERVIEW_CC SetViewMode(int nMode)
{
	if (nMode <= VIEW_PERSPECTIVE)
		g_MainProcess.SetViewMode(nMode);
	else
		g_MainProcess.SetRenderMode(nMode-VIEW_PERSPECTIVE);
}

void RENDERVIEW_CC SetLineDrawFlag(int nFlag)
{
	g_MainProcess.SetLineDrawing((nFlag) ? TRUE : FALSE);
}

void RENDERVIEW_CC RotateObject(int nDegree)
{
	g_MainProcess.RotatePickedObject();
}

void RENDERVIEW_CC ScatterObjects(int nCount)
{
	g_MainProcess.ScatterObjects(nCount);
}

void RENDERVIEW_CC RT_Render()
{
	//GI_Render();
}

void RENDERVIEW_CC RayTracingRender(HDC hdc, int nShadeType, int Nx, int Ny)
{
	g_ClsRayRender.SetShadeType(nShadeType);
	g_ClsRayRender.RT_RenderScene(hdc,Nx,Ny,TRUE);
}

void LogText(LPTSTR lpszFileName, LPTSTR lpszText)
{
	FILE*	fp;

	if (lpszFileName == NULL || lpszFileName[0] == 0)
		return;

	fp = _tfopen(lpszFileName, _T("r+t"));
	if (fp == NULL)
		fp = _tfopen(lpszFileName, _T("wt"));
	if (fp == NULL)
		return;

	fseek(fp, 0, FILE_END);
	_ftprintf(fp, _T("%s\n"), lpszText);
	fclose(fp);
}


#ifndef __RENDERVIEW_POPUP_DIALOG_HEADER__
#define __RENDERVIEW_POPUP_DIALOG_HEADER__
#pragma once

#include <windows.h>

class CPopupDialog
{
public:
	CPopupDialog();
	CPopupDialog(HWND hWndParent);

	BOOL Create(LPCTSTR lpszName, HWND hWndParent);
	BOOL Create(UINT nID, HWND hWndParent);
	virtual void Close();

	virtual void OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnCommand(UINT nNotifyCode, UINT nCtrlId, LPARAM lParam);

	virtual UINT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void SetHWnd(HWND hDlg) {m_hDialog = hDlg;}
	HWND GetHWnd() {return m_hDialog;}

	void SetControlText(UINT nID, LPTSTR lpszText);
	void GetControlText(UINT nID, LPTSTR lpszText, DWORD dwMaxLen);

	void EndDialog(int nResult);

protected:
	HWND	m_hDialog;
	HWND	m_hParent;
};


#endif	//__RENDERVIEW_POPUP_DIALOG_HEADER__
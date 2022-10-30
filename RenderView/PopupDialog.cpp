#include "StdAfx.h"
#include "PopupDialog.h"
#include "MyArray.h"
#include "Common.h"

CMyArray<CPopupDialog*>	g_DlgArray;
CPopupDialog*	g_pCurDlg;
BOOL	g_bCreated = FALSE;

INT_PTR CALLBACK PopupDialogProc(HWND, UINT, WPARAM, LPARAM);


CPopupDialog::CPopupDialog()
{
	g_pCurDlg = this;
	m_hDialog = NULL;
	m_hParent = NULL;
}

CPopupDialog::CPopupDialog(HWND hWndParent)
{
	m_hDialog = NULL;
	m_hParent = hWndParent;
}

BOOL CPopupDialog::Create(LPCTSTR lpszName, HWND hWndParent)
{
	if (lpszName == NULL)
		return FALSE;

	//Create Dialog Window
	g_bCreated	= TRUE;
	m_hParent	= hWndParent;
	m_hDialog	= CreateDialog(MyGetResourceHandle(), lpszName, m_hParent, PopupDialogProc);
	if (m_hDialog == NULL)
		return FALSE;

	//Show Dialog Window
	ShowWindow(m_hDialog, SW_SHOW);
	UpdateWindow(m_hDialog);

	return TRUE;
}

BOOL CPopupDialog::Create(UINT nID, HWND hWndParent)
{
	return Create(MAKEINTRESOURCE(nID), hWndParent);
}

void CPopupDialog::Close()
{
	//Remove from Dialog List
	CPopupDialog*	pThis = this;

	int nIndex = g_DlgArray.GetIndex(&pThis, 0, FALSE);
	g_DlgArray.Delete(nIndex, 1);
}

void CPopupDialog::OnInitDialog()
{

}

void CPopupDialog::OnOK()
{
	EndDialog(IDOK);
}

void CPopupDialog::OnCancel()
{
	EndDialog(IDCANCEL);
}

void CPopupDialog::OnCommand(UINT nNotifyCode, UINT nCtrlId, LPARAM lParam)
{
	switch (nNotifyCode)
	{
	case BN_CLICKED:
		switch (nCtrlId)
		{
		case IDOK:
			OnOK();
			break;
		case IDCANCEL:
			OnCancel();
			break;
		}
		break;
	case EN_CHANGE:
		break;
	}
}

UINT CPopupDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT nResult = 0;
	if (m_hDialog == NULL)
		return 0;

	switch (message)
	{
	case WM_INITDIALOG:
		OnInitDialog();
		break;
	case WM_DESTROY:
		Close();
		m_hDialog = NULL;
		break;
	case WM_CLOSE:
		OnCancel();
		break;
	case WM_COMMAND:
		OnCommand(HIWORD(wParam), LOWORD(wParam), lParam);
		break;
	default:
		nResult = DefWindowProc(m_hDialog, message, wParam, lParam);
	}

	return nResult;
}


void CPopupDialog::EndDialog(int nResult)
{
	if (IsWindow(m_hDialog))
		::DestroyWindow(m_hDialog);
}


//Global Function
INT_PTR CALLBACK PopupDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int x, nCount;

	//Find Dialog and Call WindowProc
	nCount = g_DlgArray.GetCount();
	for (x = 0; x < nCount; x++)
	{
		if (g_DlgArray[x]->GetHWnd() == hWnd)
			return g_DlgArray[x]->WindowProc(message, wParam, lParam);
	}

	//Not Found Dialog
	if (g_bCreated)
	{
		g_pCurDlg->SetHWnd(hWnd);
		g_DlgArray.AddItem(&g_pCurDlg, 1);
		g_bCreated = FALSE;
		return g_pCurDlg->WindowProc(message, wParam, lParam);
	}

	//Not Popup Dialogs
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void CPopupDialog::SetControlText(UINT nID, LPTSTR lpszText)
{
	if (m_hDialog == NULL || lpszText == NULL)
		return;
	SetDlgItemText(m_hDialog, nID, lpszText);
}

void CPopupDialog::GetControlText(UINT nID, LPTSTR lpszText, DWORD dwMaxLen)
{
	if (m_hDialog == NULL || lpszText == NULL)
		return;
	GetDlgItemText(m_hDialog, nID, lpszText, dwMaxLen);
}
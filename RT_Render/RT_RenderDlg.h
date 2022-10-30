#pragma once
#include "resource.h"
#include "afxwin.h"

// CRT_RenderDlg dialog

class CRT_RenderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRT_RenderDlg)

public:
	CRT_RenderDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRT_RenderDlg();

// Dialog Data
	enum { IDD = IDD_RENDER_DLG };
	int m_nShadeType;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	CStatic m_CtrlRenderStatic;
	afx_msg void OnBnClickedLambert();
	afx_msg void OnBnClickedGouraud();
	afx_msg void OnBnClickedPhong();
};

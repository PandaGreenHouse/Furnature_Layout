// RT_RenderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RT_Render.h"
#include "RT_RenderDlg.h"
#include "afxdialogex.h"

extern "C" _declspec(dllimport) void __stdcall RayTracingRender(HDC hdc, int nShadeType, int Nx, int Ny);
// CRT_RenderDlg dialog

IMPLEMENT_DYNAMIC(CRT_RenderDlg, CDialogEx)

CRT_RenderDlg::CRT_RenderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRT_RenderDlg::IDD, pParent)
{
	m_nShadeType = 0;
}

CRT_RenderDlg::~CRT_RenderDlg()
{
}

void CRT_RenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RENDER_STATIC, m_CtrlRenderStatic);
}


BEGIN_MESSAGE_MAP(CRT_RenderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_LAMBERT, &CRT_RenderDlg::OnBnClickedLambert)
	ON_BN_CLICKED(IDC_GOURAUD, &CRT_RenderDlg::OnBnClickedGouraud)
	ON_BN_CLICKED(IDC_PHONG, &CRT_RenderDlg::OnBnClickedPhong)
END_MESSAGE_MAP()


// CRT_RenderDlg message handlers


void CRT_RenderDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	RECT rect;
	m_CtrlRenderStatic.GetClientRect(&rect);
	int Nx = rect.right - rect.left;
	int Ny = rect.bottom - rect.top;
	RayTracingRender(m_CtrlRenderStatic.GetDC()->GetSafeHdc(), m_nShadeType, Nx, Ny);
	// Do not call CDialogEx::OnPaint() for painting messages
}


void CRT_RenderDlg::OnBnClickedLambert()
{
	// TODO: Add your control notification handler code here
	m_nShadeType = 0;
	Invalidate(FALSE);
}


void CRT_RenderDlg::OnBnClickedGouraud()
{
	// TODO: Add your control notification handler code here
	m_nShadeType = 1;
	Invalidate(FALSE);
}


void CRT_RenderDlg::OnBnClickedPhong()
{
	// TODO: Add your control notification handler code here
	m_nShadeType = 2;
	Invalidate(TRUE);
}

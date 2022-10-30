#include "StdAfx.h"
#include "PropertiesDlg.h"


CPropertiesDlg::CPropertiesDlg(void)
{
	m_pSrcData = NULL;
	m_TempData.InitMembers();
}


CPropertiesDlg::~CPropertiesDlg(void)
{
	m_TempData.InitMembers();
}

void CPropertiesDlg::OnInitDialog()
{
	CPopupDialog::OnInitDialog();

	if (m_hDialog == NULL)
		return;

	if (m_TempData.szId[0] != 0)
		SetControlText(IDC_EDIT_ID, m_TempData.szId);
}

void CPropertiesDlg::OnOK()
{
	TCHAR	szText[MAX_OBJECT_NAME_LEN];

	if (m_pSrcData != NULL)
	{
		szText[0] = 0;
		GetControlText(IDC_EDIT_ID, szText, MAX_OBJECT_NAME_LEN);
		if (szText[0] == 0)
		{
			MessageBox(m_hDialog, _T("Please Input ID."), _T("Confirm"), MB_OK);
			return;
		}
		_tcscpy(m_pSrcData->szId, szText);
	}

	CPopupDialog::OnOK();
}

void CPropertiesDlg::OnCancel()
{
	CPopupDialog::OnCancel();
}

void CPropertiesDlg::SetObjectNode(ObjectNode* pNode)
{
	m_pSrcData = pNode;
	m_TempData.InitMembers();
	if (pNode)
		m_TempData = *pNode;
}
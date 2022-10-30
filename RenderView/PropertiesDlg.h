#ifndef __RENDERVIEW_PROPERTIES_DLG_HEADER__
#define __RENDERVIEW_PROPERTIES_DLG_HEADER__

#pragma once

#include "PopupDialog.h"
#include "ObjectNode.h"


class CPropertiesDlg : public CPopupDialog
{
public:
	CPropertiesDlg(void);
	~CPropertiesDlg(void);

	void SetObjectNode(ObjectNode* pNode);
	void ApplyObject();

	virtual void OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

private:
	ObjectNode	m_TempData;
	ObjectNode*	m_pSrcData;
};


#endif	//__RENDERVIEW_PROPERTIES_DLG_HEADER__
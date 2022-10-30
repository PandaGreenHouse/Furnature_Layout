#include "StdAfx.h"
#include "ObjectNode.h"
#include "MyStruct.h"
#include "Model.h"

#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE


ObjectNode::ObjectNode(void)
{
	InitMembers();
}

ObjectNode::~ObjectNode()
{
	FreeMembers();
}

void ObjectNode::InitMembers()
{
	m_pParent		= NULL;
	m_pSibling		= NULL;
	m_pFirstChild	= NULL;

	m_bSelected		= false;

	memset(szId,	0, MAX_OBJECT_NAME_LEN * sizeof(TCHAR));
	memset(szName,	0, MAX_OBJECT_NAME_LEN * sizeof(TCHAR));
	memset(szCode,	0, MAX_OBJECT_NAME_LEN * sizeof(TCHAR));
	memset(szModel, 0, MAX_OBJECT_NAME_LEN * sizeof(TCHAR));

	x = y = z = 0;
	w = d = h = 0;
	xa= ya= za= 0;

	isMove = isDelete = rotateMaterial = bReserved = 0;
	putMode		= XML_PM_CUBIC;
	nMatIndex	= -1;

	memset(&matWorld, 0, sizeof(D3DXMATRIX));
	memset(&matRotation, 0, sizeof(D3DXMATRIX));
	pVertexBuffer	= NULL;
	pIndexBuffer	= NULL;
	pLineVertexBuffer = NULL;
	pLineIndexBuffer = NULL;
}

void ObjectNode::FreeMembers()
{
	if (pVertexBuffer)
	{
		pVertexBuffer->Release();
		pVertexBuffer = NULL;
	}

	if (pIndexBuffer)
	{
		pIndexBuffer->Release();
		pIndexBuffer = NULL;
	}

	if(pLineVertexBuffer)
	{
		pLineVertexBuffer->Release();
		pLineVertexBuffer = NULL;
	}

	if(pLineIndexBuffer)
	{
		pLineIndexBuffer->Release();
		pLineIndexBuffer = NULL;
	}
}

void ObjectNode::setSelected(bool bSelected)
{
	ObjectNode*	pChild;

	//Apply to This Object
	m_bSelected = bSelected;
	//Apply to Children
	if (m_pFirstChild)
	{
		//---First Child
		m_pFirstChild->setSelected(bSelected);
		//---First Child's siblings
		pChild = m_pFirstChild->getSibling();
		while (pChild)
		{
			//Apply to Sibling
			pChild->setSelected(bSelected);
			//Get next Sibling
			pChild = pChild->getSibling();
		}
	}
}

void ObjectNode::setXmlAttributes(void* pDoc, void* pElem)
{
	LPXML_MATINFO	pMat = NULL;
	TCHAR			szText[XML_MAX_NAME_LEN], *pValue;

	xercesc::DOMDocument*	xmlDoc = (xercesc::DOMDocument*) pDoc;
	DOMElement*	xmlThis = (DOMElement*) pElem, *xmlChild = NULL;

	if (xmlDoc == NULL || xmlThis == NULL)
		return;
	
//Set Data to DOMElement
	if (nMatIndex >= 0)
		pMat = GetMaterialObject(nMatIndex);

	xmlThis->setAttribute((const XMLCh*) _T("id"),	szId);
	xmlThis->setAttribute(_T("name"), (const XMLCh*) szName);
	xmlThis->setAttribute(_T("code"), (const XMLCh*) szCode);
	xmlThis->setAttribute((const XMLCh*) _T("model"), (const XMLCh*) szModel);

	if (isMove)
		_tcscpy(szText, _T("true"));
	else
		_tcscpy(szText, _T("false"));
	xmlThis->setAttribute((const XMLCh*) _T("isMove"), (const XMLCh*) szText);
	if (isDelete)
		_tcscpy(szText, _T("true"));
	else
		_tcscpy(szText, _T("false"));
	xmlThis->setAttribute((const XMLCh*) _T("isDelete"), (const XMLCh*) szText);
	if (rotateMaterial)
		_tcscpy(szText, _T("true"));
	else
		_tcscpy(szText, _T("false"));
	xmlThis->setAttribute((const XMLCh*) _T("rotateMaterial"), (const XMLCh*) szText);
	
	_stprintf(szText, _T("%f"), x);
	xmlThis->setAttribute((const XMLCh*) _T("x"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), y);
	xmlThis->setAttribute((const XMLCh*) _T("y"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), z);
	xmlThis->setAttribute((const XMLCh*) _T("z"), (const XMLCh*) szText);

	_stprintf(szText, _T("%f"), w);
	xmlThis->setAttribute((const XMLCh*) _T("w"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), d);
	xmlThis->setAttribute((const XMLCh*) _T("d"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), h);
	xmlThis->setAttribute((const XMLCh*) _T("h"), (const XMLCh*) szText);

	_stprintf(szText, _T("%f"), xa);
	xmlThis->setAttribute((const XMLCh*) _T("xa"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), ya);
	xmlThis->setAttribute((const XMLCh*) _T("ya"), (const XMLCh*) szText);
	_stprintf(szText, _T("%f"), za);
	xmlThis->setAttribute((const XMLCh*) _T("za"), (const XMLCh*) szText);

	pValue = GetPutModeString(putMode);
	if (pValue)
		xmlThis->setAttribute((const XMLCh*) _T("putMode"), (const XMLCh*) pValue);

	if (pMat)
		xmlThis->setAttribute((const XMLCh*) _T("material"), (const XMLCh*) pMat->szImagePath);


//Set Children Attributes
	ObjectNode*	pChild;

	pChild = m_pFirstChild;
	while (pChild)
	{
		pValue = GetObjectXmlTagName(pChild->nType);
		if (pValue)
		{
			xmlChild = xmlDoc->createElement(pValue);
			pChild->setXmlAttributes(pDoc, xmlChild);
			xmlThis->appendChild(xmlChild);
		}

		pChild = pChild->getSibling();
	}
}

bool ObjectNode::setAloneColor()
{
	LPXML_MATINFO	pMat;
	LPVERTEX	pVerts = NULL;
	DWORD	color;
	int		x;

	if (nMatIndex < 0 || nMatIndex >= g_arrMats.GetCount())
		return false;
	pMat = g_arrMats.GetItem(nMatIndex);

	if (pVertexBuffer == NULL)
		return false;

	if ((pVertexBuffer)->Lock( 0, 0, (void**)&pVerts, D3DLOCK_DISCARD ) != D3D_OK)
		return false;

	color = D3DCOLOR_RGBA((DWORD) pMat->fDiffuseColor[0], (DWORD) pMat->fDiffuseColor[1], (DWORD) pMat->fDiffuseColor[2], 255);
	for (x = 0; x < intVtCount; x++)
	{
		pVerts[x].Diffuse	= color;
		pVerts[x].Specular	= D3DCOLOR_RGBA(255, 255, 255, 255);
	}
	(pVertexBuffer)->Unlock();

	return true;
}
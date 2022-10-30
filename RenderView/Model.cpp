#include "stdafx.h"
#include "RenderView.h"
#include "Model.h"


#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

// Use the Document Object Model (DOM) API
#include <xercesc/dom/DOM.hpp>
// Required for outputing a Xerces DOMDocument
// to a standard output stream (Also see: XMLFormatTarget)
#include <xercesc/framework/StdOutFormatTarget.hpp>

// Required for outputing a Xerces DOMDocument
// to the file system (Also see: XMLFormatTarget)
#include <xercesc/framework/LocalFileFormatTarget.hpp>



XERCES_CPP_NAMESPACE_USE


TCHAR	g_szXmlTagNames[XML_OT_MAX_COUNT][XML_MAX_NAME_LEN] = {_T(""), _T("group"), _T("chest"), _T("module"),
												_T("plank"), _T("hole"), _T("hardware"), _T("groove")};

enum
{
	XML_ATTR_TYPE_UNKNOWN = -1,
	XML_ATTR_TYPE_ID = 0,
	XML_ATTR_TYPE_NAME,
	XML_ATTR_TYPE_CODE,
	XML_ATTR_TYPE_MODEL,
	XML_ATTR_TYPE_ISMOVE,
	XML_ATTR_TYPE_ISDELETE,
	XML_ATTR_TYPE_XPOS,
	XML_ATTR_TYPE_YPOS,
	XML_ATTR_TYPE_ZPOS,
	XML_ATTR_TYPE_WIDTH,
	XML_ATTR_TYPE_DEPTH,
	XML_ATTR_TYPE_HEIGHT,
	XML_ATTR_TYPE_XROT,
	XML_ATTR_TYPE_YROT,
	XML_ATTR_TYPE_ZROT,
	XML_ATTR_TYPE_PUTMODE,
	XML_ATTR_TYPE_MATERIAL,
	XML_ATTR_TYPE_ROTATE_MATERIAL,

	XML_ATTR_TYPE_MAX,
};
TCHAR	g_szXmlAttrNames[XML_ATTR_TYPE_MAX][XML_MAX_NAME_LEN] = {_T("id"), _T("name"), _T("code"), _T("model"), _T("isMove"),
											_T("isDelete"), _T("x"), _T("y"), _T("z"), _T("w"), _T("d"), _T("h"),
											_T("xa"), _T("ya"), _T("za"), _T("putMode"), _T("material"), _T("rotateMaterial")};
TCHAR	g_szPutModes[XML_PM_MAX_COUNT][XML_MAX_NAME_LEN] = {_T("横放"), _T("竖放"), _T("平放")};

int		g_nCurIndex[XML_OT_MAX_COUNT] = {0, -1, -1, -1, -1, -1, -1, -1};	//Current Element Index of the Array

TCHAR	g_szModelDir[XML_MAX_PATH_LEN];	

CMyArray<XML_MATINFO>	g_arrMats;			//Material Info Array
CMyArray<ObjectNode*>	g_arrObjects;		//Object Pointer Array
CMyArray<int>			g_arrChildCnt;		//Child Count Array to Build Tree Structure

int  Parse_GetTypeFromTagName(LPTSTR lpszKeyName);
int  Parse_GetAttributeType(LPTSTR lpszName);

//Return Value:		0 - false, 1 - true
int  Parse_BooleanFromString(LPTSTR lpszValue, int nDefault);
int  Parse_ConstantFromString(LPTSTR lpszValue, LPTSTR lpszTable, int nTableSize, int nUnitLen, int nDefault, BOOL bCaseSensitive);

void Parse_StartElement(LPTSTR lpszKeyName, const Attributes& attributes);
void Parse_EndElement(LPTSTR lpszKeyName);

void GetDirFromPath(LPCTSTR lpszFile, LPTSTR lpszDir, int nMaxDirLen);

class XmlModelParserHandler : public DefaultHandler
{
public:
	XmlModelParserHandler()	{}
	~XmlModelParserHandler()	{}

	void fatalError(const SAXParseException& e)
	{
		TCHAR	szText[100];

		_stprintf(szText, _T("Fatal Error at file : %s, line: %d, char: %d \n Message: %s"),
					e.getSystemId(), e.getLineNumber(), e.getColumnNumber(), e.getMessage());
		OutputDebugString(szText);
	}

	void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attributes)
	{
		LPTSTR lpszKeyName = NULL;

		if (localname && localname[0] != 0)
			lpszKeyName = (LPTSTR) localname;
		else if (qname && qname[0] != 0)
			lpszKeyName = (LPTSTR) qname;
		Parse_StartElement(lpszKeyName, attributes);
	}

	void endElement( const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname )
	{
		LPTSTR lpszKeyName = NULL;

		if (localname && localname[0] != 0)
			lpszKeyName = (LPTSTR) localname;
		else if (qname && qname[0] != 0)
			lpszKeyName = (LPTSTR) qname;
		Parse_EndElement(lpszKeyName);
	}
};

void Parse_StartElement(LPTSTR lpszKeyName, const Attributes& attributes)
{
	int nType = XML_OT_UNKNOWN, x;
	LPTSTR		lpszValue, lpszName, lpszTemp;
	ObjectNode*	pObj = NULL;
	XML_MATINFO	mat;

	//Get Type and Check
	nType = Parse_GetTypeFromTagName(lpszKeyName);
	if (nType == XML_OT_UNKNOWN)
		return;

	pObj = new ObjectNode();
	if (pObj == NULL)
		return;

	//Current Type Index
	g_nCurIndex[nType] = g_arrObjects.GetCount();
	//Set Object Type
	pObj->nType		= nType;
	//---Index for Empty Material
	pObj->nMatIndex	= -1;
	
	for (x = 0; x < (int) attributes.getLength(); x++)
	{
		lpszName = (LPTSTR) attributes.getLocalName(x);
		if (lpszName == NULL || lpszName[0] == 0)
			lpszName = (LPTSTR) attributes.getQName(x);

		lpszValue = (LPTSTR) attributes.getValue(x);
		if (lpszValue == NULL)
			continue;

		switch (Parse_GetAttributeType(lpszName))
		{
		case XML_ATTR_TYPE_ID:			//---'id' attribute
			_tcscpy(pObj->szId, lpszValue);
			break;

		case XML_ATTR_TYPE_NAME:		//---'name' attribute
			_tcscpy(pObj->szName, lpszValue);
			break;

		case XML_ATTR_TYPE_CODE:		//---'code' attribute	
			_tcscpy(pObj->szCode, lpszValue);
			break;

		case XML_ATTR_TYPE_MODEL:		//---'model' attribute
			_tcscpy(pObj->szModel, lpszValue);
			break;

		case XML_ATTR_TYPE_ISMOVE:		//---'isMove' attribute
			pObj->isMove = (BYTE) Parse_BooleanFromString(lpszValue, 0);
			break;

		case XML_ATTR_TYPE_ISDELETE:	//---'isDelete' attribute
			pObj->isDelete = (BYTE) Parse_BooleanFromString(lpszValue, 0);
			break;

		case XML_ATTR_TYPE_XPOS:		//---'x' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->x));
			pObj->vOrg.x = pObj->x;
			break;

		case XML_ATTR_TYPE_YPOS:		//---'y' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->y));
			pObj->vOrg.y = pObj->y;
			break;

		case XML_ATTR_TYPE_ZPOS:		//---'z' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->z));
			pObj->vOrg.z = pObj->z;
			break;

		case XML_ATTR_TYPE_WIDTH:		//---'w' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->w));
			break;

		case XML_ATTR_TYPE_DEPTH:		//---'d' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->d));
			break;

		case XML_ATTR_TYPE_HEIGHT:		//---'h' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->h));
			break;

		case XML_ATTR_TYPE_XROT:		//---'xa' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->xa));
			pObj->xa = (D3DX_PI * pObj->xa) / 180;
			break;

		case XML_ATTR_TYPE_YROT:		//---'ya' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->ya));
			pObj->ya = (D3DX_PI * pObj->ya) / 180;
			break;

		case XML_ATTR_TYPE_ZROT:		//---'za' attribute
			_stscanf(lpszValue, _T("%f"), &(pObj->za));
			pObj->za = (D3DX_PI * pObj->za) / 180;
			break;

		case XML_ATTR_TYPE_PUTMODE:		//---'putMode' attribute
			pObj->putMode = Parse_ConstantFromString(lpszValue, (LPTSTR) g_szPutModes,
								XML_PM_MAX_COUNT, XML_MAX_NAME_LEN, XML_PM_CUBIC, FALSE);
			break;

		case XML_ATTR_TYPE_MATERIAL:	//---'material' attribute
			if (lpszValue[0] != 0)
			{
				//Init Mat info for search
				memset(&mat, 0, sizeof(XML_MATINFO));
				lpszTemp = _tcschr(lpszValue, '@');
				if (lpszTemp)
				{
					mat.nType = XML_MT_COLOR;
					if (_stscanf(lpszTemp + 2, _T("%f, %f, %f"), &(mat.fDiffuseColor[0]), &(mat.fDiffuseColor[1]), &(mat.fDiffuseColor[2])) == 3)
					{
						mat.fDiffuseColor[3] = 1.0f;
						memcpy(&(mat.fSpecularColor), &(mat.fDiffuseColor), 4 * sizeof(float));
					} else
					{
						mat.nType = XML_MT_IMAGE;
						_tcscpy(mat.szImagePath, lpszValue);
					}
				} else
				{
					mat.nType = XML_MT_IMAGE;
					_tcscpy(mat.szImagePath, lpszValue);
				}
				
				//set Mat Index for search of Material Array
				pObj->nMatIndex = g_arrMats.GetIndex(&mat, 0, TRUE);
			}
			break;

		case XML_ATTR_TYPE_ROTATE_MATERIAL:	//---'rotateMaterial' attribute
			pObj->rotateMaterial = (BYTE) Parse_BooleanFromString(lpszValue, 0);
			break;
		} 
	}

	//Set Start Children Index to Array
	x = g_nCurIndex[nType] + 1;	
	g_arrChildCnt.AddItem(&x, 1);

	//Add Object to array
	g_arrObjects.AddItem(&pObj, 1);
}

void Parse_EndElement(LPTSTR lpszKeyName)
{
	int nType = XML_OT_UNKNOWN;
	int* pnChilds = NULL;
	ObjectNode* pCur = NULL;

	//Get Type and Check
	nType = Parse_GetTypeFromTagName(lpszKeyName);
	if (nType == XML_OT_UNKNOWN)
		return;

	//Get Current Object Index for the Type
	pCur	= g_arrObjects[g_nCurIndex[nType]];
	pnChilds= g_arrChildCnt.GetItem(g_nCurIndex[nType]);

	//Set Children Info
	if (*pnChilds >= g_arrObjects.GetCount())
		*pnChilds = 0;
	else
		*pnChilds = g_arrObjects.GetCount() - *pnChilds;
}

int Parse_GetAttributeType(LPTSTR lpszName)
{
	int nType = XML_ATTR_TYPE_UNKNOWN;

	if (lpszName == NULL || lpszName[0] == 0)
		return XML_ATTR_TYPE_UNKNOWN;

	for (nType = 0; nType < XML_ATTR_TYPE_MAX; nType++)
	{
		if (_tcscmp(lpszName, g_szXmlAttrNames[nType]) == 0)
			break;
	}

	if (nType >= XML_ATTR_TYPE_MAX)
		nType = XML_ATTR_TYPE_UNKNOWN;

	return nType;
}

//Return Value:		0 - false, 1 - true
int  Parse_BooleanFromString(LPTSTR lpszValue, int nDefault)
{
	TCHAR	szText[64];
	int nValue = nDefault;
	if (lpszValue == NULL || lpszValue[0] == 0)
		return nDefault;

	//Convert Value to lower case
	_tcscpy(szText, lpszValue);
	_tcslwr(szText);

	//Get Value and Return
	if (_tcscmp(szText, _T("true")) == 0)
		nValue = 1;
	else if (_tcscmp(szText, _T("false")) == 0)
		nValue = 0;
	else
		nValue = nDefault;

	return nValue;
}

//Get a constant value(for example a putMode value) from string
//lpszValue - string to find
//lpszTable - constant string array
//nTableSize - size of lpszTable
//nDefault - Default Value
//bCaseSensitive - if FALSE value string must converted into Lower case to Compare String with Table Elements
int  Parse_ConstantFromString(LPTSTR lpszValue, LPTSTR lpszTable, int nTableSize, int nUnitLen, int nDefault, BOOL bCaseSensitive)
{
	TCHAR	szText[64], *pCur;
	int nResult = nDefault;
	
	_tcscpy(szText, lpszValue);
	if (!bCaseSensitive)
		_tcslwr(szText);
	pCur = lpszTable;
	for (int x = 0; x < nTableSize; x++, pCur += nUnitLen)
	{
		if (_tcscmp(szText, pCur) == 0)
		{
			nResult = x;
			break;
		}
	}

	return nResult;
}


int  Parse_GetTypeFromTagName(LPTSTR lpszKeyName)
{
	TCHAR	szCompare[XML_MAX_NAME_LEN];
	int		nType;

	//Copy Key Name and Convert into Lower Case
	_tcscpy(szCompare, lpszKeyName);
	_tcslwr(szCompare);

	//Compare all Key Names and Find Type
	for (nType = 0; nType < XML_OT_MAX_COUNT; nType++)
	{
		if (_tcscmp(szCompare, g_szXmlTagNames[nType]) == 0)
			break;
	}

	if (nType >= XML_OT_MAX_COUNT)
		nType = XML_OT_UNKNOWN;
	return nType;
}

void BuildObjectTree(int nStart, int nCount)
{
	int			nIndex, nTemp, *pnChildCnt;
	ObjectNode	*pParent, *pCurChild, *pSibling;

	if (nStart < 1 || ((nStart + nCount) > g_arrObjects.GetCount()))
		return;

	//Get Parent and Child Count
	pParent		= g_arrObjects[nStart - 1];
	pnChildCnt	= g_arrChildCnt.GetItem(0);

	//Get Current Object and Set Parent
	pCurChild = g_arrObjects[nStart];
	pParent->setChild(pCurChild);
	pCurChild->setParent(pParent);

	//Get First Child of This Object and Set Parent
	nIndex	= nStart;

	//Add Children
	while (nCount)
	{
		//---Add Children
		if (pnChildCnt[nIndex] > 0)
			BuildObjectTree(nIndex + 1, pnChildCnt[nIndex]);

		//---Decrease Count and Increase Child Count to find Next Sibling
		nTemp	= pnChildCnt[nIndex] + 1;
		nCount -= nTemp;
		nIndex += nTemp;
		if (nCount < 1)
			break;

		//---Next Sibling
		pSibling = g_arrObjects[nIndex];
		pCurChild->setSibling(pSibling);
		pSibling->setParent(pCurChild->getParent());
		pCurChild = pSibling;
	}
}

//Get World Transform Matrix of Node
void GetWorldTransformMatrix(ObjectNode* pNode)
{
	ObjectNode*		pChild;
	D3DXMATRIX		rotMat[4];
	D3DXVECTOR3		vRotate, vTrans;
	D3DXQUATERNION	QRotation[3];

	if (pNode == NULL)
		return;

//Get Matrix of This Object
	//---Get Rotate Vector
	vRotate.x	= pNode->xa;
	vRotate.y	= pNode->ya;
	vRotate.z	= pNode->za;
	//---Get Translate Vector
	vTrans.x	= pNode->x;
	vTrans.y	= pNode->y;
	vTrans.z	= pNode->z;

	//---Rotate around x axis
	QRotation[0].x = sin(vRotate.x/2);
	QRotation[0].y = 0.0f;
	QRotation[0].z = 0.0f;
	QRotation[0].w = cos(vRotate.x/2);
	D3DXMatrixTransformation( &rotMat[0],
		NULL,//vScaleCenter,
		NULL,
		NULL,//vScaling,
		NULL,//Center of rotation
		&QRotation[0],// rotation axis, rotation angle 
		NULL //translation vector
		);

	//---Rotate around y axis
	QRotation[1].x = 0.0f;
	QRotation[1].y = sin(vRotate.y/2);
	QRotation[1].z = 0.0f;
	QRotation[1].w = cos(vRotate.y/2);
	D3DXMatrixTransformation( &rotMat[1],
		NULL,//vScaleCenter,
		NULL,
		NULL,//vScaling,
		NULL,//Center of rotation
		&QRotation[1],// rotation axis, rotation angle 
		NULL //translation vector
		);

	//---Rotate around z axis
	QRotation[2].x = 0.0f;
	QRotation[2].y = 0.0f;
	QRotation[2].z = sin(vRotate.z/2);
	QRotation[2].w = cos(vRotate.z/2);
	D3DXMatrixTransformation( &rotMat[2],
		NULL,//vScaleCenter,
		NULL,
		NULL,//vScaling,
		NULL,//Center of rotation
		&QRotation[2],// rotation axis, rotation angle 
		NULL //translation vector
		);

	//---Get Rotate Matrix
	D3DXMatrixMultiply(&rotMat[3],&rotMat[0],&rotMat[1]);
	D3DXMatrixMultiply(&rotMat[3],&rotMat[2],&rotMat[3]);
	pNode->matRotation = rotMat[3];

	//---Get Translate
	D3DXMATRIX transMat;
	D3DXMatrixTransformation(
		&transMat,
		NULL,//&transform.vScaleCenter,
		NULL,
		NULL,//&transform.vScaling,
		NULL,//Center of rotation
		NULL,// rotation axis, rotation angle 
		&vTrans //translation vector
		);

	//---World Transform Matrix
	D3DXMATRIX worldMat;
	D3DXMatrixIdentity(&worldMat);
	D3DXMatrixMultiply(&worldMat, &rotMat[3], &transMat);
	D3DXMatrixMultiply(&worldMat, &worldMat, &(pNode->matWorld));
	pNode->matWorld = worldMat;

//Get Matrix of Child Nodes
	pChild = pNode->getChild();
	while (pChild)
	{
		//---Get Matrix of Current Sibling
		pChild->matWorld	= pNode->matWorld;
		pChild->matRotation	= pNode->matRotation;
		GetWorldTransformMatrix(pChild);
		pChild = pChild->getSibling();
	}
}

void GetObjectsInfo()
{
	ObjectNode*	pRoot = NULL;

	if (g_arrObjects.GetCount() < 1 || g_arrObjects[0] == NULL)
		return;

	//Get Root Object
	pRoot = g_arrObjects[0];

	//Load Identical Matrix
	D3DXMatrixIdentity(&(pRoot->matWorld));
	pRoot->matRotation = pRoot->matWorld;

	//World Transform Matrix of All Objects
	GetWorldTransformMatrix(pRoot);

	//Get Applied Object count of materials
	for (int x = 0; x < g_arrObjects.GetCount(); x++)
	{
		pRoot = g_arrObjects[x];
		if (pRoot->nMatIndex >= 0)
			g_arrMats[pRoot->nMatIndex].nAppliedObjects++;
	}
}

int  RENDERVIEW_CC LoadModel(LPCTSTR lpszFileName, LPCTSTR lpszContent)
{
	TCHAR	szError[128];
	LPXML_MATINFO	pMat;

	if (lpszFileName == NULL || lpszFileName[0] == 0)
	{
		if ((lpszContent != NULL) && (lpszContent[0] != 0))
			return LoadModelByContent(lpszContent);

		return 0;
	}

	if (g_arrMats.GetCount() > 1)
	{
		pMat = g_arrMats.GetItem(0);
		for (int x = 0; x < g_arrMats.GetCount(); x++, pMat++)
		{
			pMat->fDiffuseColor[0] = 0.0f;
			pMat->fDiffuseColor[1] = 255.0f;
			pMat->fDiffuseColor[2] = 0.0f;
			if (pMat->pTexture)
			{
				pMat->pTexture->Release();
				pMat->pTexture = NULL;
			}
		}
		g_arrMats.Clear();
	}
	g_arrObjects.Clear();
	g_arrChildCnt.Clear();
	g_arrMats.Create(64, 16);
	g_arrObjects.Create(64, 16);
	g_arrChildCnt.Create(64, 16);

	//Initialize XML4C2 system
	try
	{
		XMLPlatformUtils::Initialize();
	} catch (const XMLException&) {}

	//Create Parser
	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	XmlModelParserHandler	handler;

	//Start Parsing
	try
	{
		parser->setContentHandler(&handler);
		parser->setErrorHandler(&handler);
		parser->parse((const XMLCh*) lpszFileName);
	} catch (const XMLException& e)
	{
		_stprintf(szError, _T("XMLException: "), e.getMessage());
	}
	catch (const SAXParseException& e)
	{
		_stprintf(szError, _T("SAXParseException: "), e.getMessage());
		OutputDebugString(szError);
	} catch (...)
	{
		OutputDebugString(_T("Unknown Error"));
	}

	//Delete Parser and Terminate XML utilities
	delete parser;
	XMLPlatformUtils::Terminate();

	//Build Tree of Objects and Object Informations(Transform Matrix etc.)
	BuildObjectTree(1, g_arrChildCnt[0]);
	GetObjectsInfo();
	//Save Model Directory Path
	GetDirFromPath(lpszFileName, g_szModelDir, XML_MAX_PATH_LEN);
	//Load Model Vertex and Faces for Rendering
	g_MainProcess.LoadXmlModel();

	return 1;
}

int  RENDERVIEW_CC LoadModelByContent(LPCTSTR lpszContent)
{
	TCHAR	szError[128];
	LPXML_MATINFO	pMat;

	if (g_arrMats.GetCount() > 1)
	{
		pMat = g_arrMats.GetItem(0);
		for (int x = 0; x < g_arrMats.GetCount(); x++, pMat++)
		{
			if (pMat->pTexture)
			{
				pMat->pTexture->Release();
				pMat->pTexture = NULL;
			}
		}
		g_arrMats.Clear();
	}
	g_arrObjects.Clear();
	g_arrChildCnt.Clear();
	g_arrMats.Create(64, 16);
	g_arrObjects.Create(64, 16);
	g_arrChildCnt.Create(64, 16);

	//Initialize XML4C2 system
	try
	{
		XMLPlatformUtils::Initialize();
	} catch (const XMLException&) {}

	//Create Parser
	xercesc::MemBufInputSource	myxml_buf((const XMLByte*) lpszContent, sizeof(TCHAR) * (_tcslen(lpszContent) + 1), (const XMLCh*) _T("xml_mem_buf1"));
	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	XmlModelParserHandler	handler;

	//Start Parsing
	try
	{
		parser->setContentHandler(&handler);
		parser->setErrorHandler(&handler);
		parser->parse(myxml_buf);
	} catch (const XMLException& e)
	{
		_stprintf(szError, _T("XMLException: "), e.getMessage());
	}
	catch (const SAXParseException& e)
	{
		_stprintf(szError, _T("SAXParseException: "), e.getMessage());
		OutputDebugString(szError);
	} catch (...)
	{
		OutputDebugString(_T("Unknown Error"));
	}

	//Delete Parser and Terminate XML utilities
	delete parser;
	XMLPlatformUtils::Terminate();

	//Build Tree of Objects and Object Informations(Transform Matrix etc.)
	BuildObjectTree(1, g_arrChildCnt[0]);
	GetObjectsInfo();
	//Save Model Directory Path
	//GetDirFromPath(lpszContent, g_szModelDir, XML_MAX_PATH_LEN);
	//Load Model Vertex and Faces for Rendering
	g_MainProcess.LoadXmlModel();


	return 1;
}


LPXML_MATINFO GetMaterialObject(int nObject)
{
	if ((nObject >= 0) || (nObject < g_arrMats.GetCount()))
		return g_arrMats.GetItem(nObject);

	return NULL;
}

TCHAR* GetPutModeString(int nIndex)
{
	if ((nIndex > XML_PM_UNKNOWN) || (nIndex < XML_PM_MAX_COUNT))
		return g_szPutModes[nIndex];

	return NULL;
}

TCHAR* GetObjectXmlTagName(int nType)
{
	if ((nType > XML_OT_UNKNOWN) || (nType < XML_OT_MAX_COUNT))
		return g_szXmlTagNames[nType];

	return NULL;
}

int  RENDERVIEW_CC SaveModel(LPCTSTR lpszFileName)
{
	ObjectNode*	pRoot;

	if (lpszFileName == NULL || lpszFileName[0] == 0 || g_arrObjects.GetCount() < 1)
		return 0;

	pRoot = *(g_arrObjects.GetItem(0));
	if (pRoot == NULL)
		return 0;

	// Initialize Xerces.
	XMLPlatformUtils::Initialize();

	//Create DOM Implementation Object
	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(_T("Core"));

	//Create DOM Document
	xercesc::DOMDocument* doc = impl->createDocument(0, (const XMLCh*) g_szXmlTagNames[XML_OT_CHEST], 0);  //root element namespace URI, root element name, document type object (0: DTD)

	//Set Attributes of Elements
	DOMElement* rootElem = doc->getDocumentElement();
	pRoot->setXmlAttributes(doc, rootElem);

	XMLFormatTarget	*myFormTarget;
	DOMLSSerializer	*theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMLSOutput		*theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();

	myFormTarget = new LocalFileFormatTarget((const XMLCh*) lpszFileName);
	theOutputDesc->setByteStream(myFormTarget);
	// set user specified output encoding
	theOutputDesc->setEncoding(XMLString::transcode("UTF-8"));

	theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMXMLDeclaration, true);

	theSerializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
	theSerializer->write(doc, theOutputDesc);

	myFormTarget->flush();

	delete myFormTarget;
	
	theOutputDesc->release();
	theSerializer->release();

	XMLPlatformUtils::Terminate();

	return 1;
}

void GetDirFromPath(LPCTSTR lpszFile, LPTSTR lpszDir, int nMaxDirLen)
{
	int nLen = 0;
	LPTSTR	pFind;

	if (lpszDir == NULL || nMaxDirLen < 1)
		return;

	//Get Current Directory
	GetCurrentDirectory(nMaxDirLen, lpszDir);
	nLen = _tcslen(lpszDir) - 1;
	if ((lpszDir[nLen] != '\\') && (lpszDir[nLen] != '/'))
	{
		lpszDir[nLen] = '\\';
		lpszDir[nLen+1] = 0;
		nLen++;
	}

	if (lpszFile == NULL || lpszFile[0] == 0)
		return;

	//If Source file path is not an absolute path, append from current directory
	pFind = (LPTSTR) _tcschr(lpszFile, ':');
	if (pFind == NULL)
	{
		//If there is no slash or backslash at first of file path, append backslash to Target Directory Path
		if ((lpszFile[0] != '\\') && (lpszFile[0] != '/'))
		{
			lpszDir[nLen] = '\\';
			lpszDir[nLen+1] = 0;
			nLen++;
		}
	} else
		nLen = 0;

	//Add Source File Path to Target Directory Path
	_tcscpy(lpszDir + nLen, lpszFile);

	//Find BackSlash from the End of Target Directory Path
	pFind = _tcsrchr(lpszDir + nLen, '\\');
	if (pFind == NULL)
		pFind = _tcsrchr(lpszDir + nLen, '/');

	//Slash Processing of Target Directory Path
	if (pFind == NULL)
		lpszDir[nLen] = 0;
	else
		pFind[1] = 0;
}

int  GetImageAbsolutePath(LPTSTR lpszAbsPath, LPTSTR lpszFile)
{
	int nLen = 0;
	LPTSTR	pFind;

	if (lpszAbsPath == NULL || lpszFile == NULL || lpszFile[0] == 0)
		return FALSE;

	//Copy Model Directory Path to Absolute Path
	_tcscpy(lpszAbsPath, g_szModelDir);
	nLen = _tcslen(lpszAbsPath);

	//If Source file path is not an absolute path, append from current directory
	pFind = _tcschr(lpszFile, ':');
	if (pFind != NULL)
		nLen = 0;

	//Add Source File Path to Absolute Path
	_tcscpy(lpszAbsPath + nLen, lpszFile);

	nLen = _tcslen(lpszAbsPath);

	return nLen;
}

bool CheckFileExists(LPTSTR lpszPath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (lpszPath == NULL || lpszPath[0] == 0)
		return false;

	hFile = CreateFile(lpszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(hFile);
	return true;
}

int  RENDERVIEW_CC GetMaterialCount()
{
	return g_arrMats.GetCount();
}

int  RENDERVIEW_CC GetMaterialInfo(int nIndex, LPOUT_MATINFO pMat)
{
	LPXML_MATINFO	pSrc = NULL;

	if (nIndex < 0 || nIndex >= g_arrMats.GetCount() || pMat == NULL)
		return 0;

	//Get Item
	pSrc = g_arrMats.GetItem(nIndex);

	//Get Type
	pMat->nType = pSrc->nType;

	//Get absolute Path of Image
	GetImageAbsolutePath(pMat->szImagePath, pSrc->szImagePath);

	//Get Color Info
	memcpy(pMat->fAmbientColor, pSrc->fAmbientColor, 4 * sizeof(float));
	memcpy(pMat->fDiffuseColor, pSrc->fDiffuseColor, 4 * sizeof(float));
	memcpy(pMat->fSpecularColor, pSrc->fSpecularColor, 4 * sizeof(float));

	return 1;
}

int  RENDERVIEW_CC AddMaterial(LPOUT_MATINFO pMat)
{
	XML_MATINFO	mat;

	//Init Material
	memset(&mat, 0, sizeof(XML_MATINFO));
	//Copy Data from Source
	if (pMat != NULL)
	{
		//---Type
		mat.nType = pMat->nType;
		//---Image Path
		GetImageAbsolutePath(mat.szImagePath, pMat->szImagePath);
		//---Color info
		memcpy(mat.fAmbientColor, pMat->fAmbientColor, 4 * sizeof(float));
		memcpy(mat.fDiffuseColor, pMat->fDiffuseColor, 4 * sizeof(float));
		memcpy(mat.fSpecularColor, pMat->fSpecularColor, 4 * sizeof(float));
	}

	return (g_arrMats.AddItem(&mat) ? 1 : 0);
}

int  RENDERVIEW_CC EditMaterial(int nIndex, LPOUT_MATINFO pMat)
{
	LPXML_MATINFO	pSrc = NULL;

	if (nIndex < 0 || nIndex >= g_arrMats.GetCount() || pMat == NULL)
		return 0;

	//Get Item to Edit
	pSrc = g_arrMats.GetItem(nIndex);
	//Copy Data From Source
	//---Type
	pSrc->nType = pMat->nType;
	//---Image Path
	GetImageAbsolutePath(pSrc->szImagePath, pMat->szImagePath);
	//---Color info
	memcpy(pSrc->fAmbientColor, pMat->fAmbientColor, 4 * sizeof(float));
	memcpy(pSrc->fDiffuseColor, pMat->fDiffuseColor, 4 * sizeof(float));
	memcpy(pSrc->fSpecularColor, pMat->fSpecularColor, 4 * sizeof(float));

	return 1;
}

//Return Values: 0 - Delete failed, 1 - Delete Success	2 - Cannot Delete because some objects use this material
int  RENDERVIEW_CC DeleteMaterial(int nIndex)
{
	ObjectNode*		pObj = NULL;
	LPXML_MATINFO	pMat = NULL;

	if (nIndex < 0 || nIndex >= g_arrMats.GetCount())
		return 0;

	pMat = g_arrMats.GetItem(nIndex);
	if (pMat->nAppliedObjects > 0)
		return 2;

	return (g_arrMats.Delete(nIndex) ? 1 : 0);
}

int  RENDERVIEW_CC ApplyTexture(int nObject, int nMatIdx)
{
	int nResult = 0, x;
	ObjectNode* pRoot = NULL;

	if (nMatIdx < 0 || nMatIdx >= g_arrMats.GetCount())
		return 0;

	//Apply Material
	nResult = (g_MainProcess.SetTexture(nMatIdx) ? 1 : 0);

	//Recalculate Applied Object Count Field of Materials
	for (x = 0; x < g_arrMats.GetCount(); x++)
		g_arrMats[x].nAppliedObjects = 0;
	for (x = 0; x < g_arrObjects.GetCount(); x++)
	{
		pRoot = g_arrObjects[x];
		if (pRoot->nMatIndex >= 0)
			g_arrMats[pRoot->nMatIndex].nAppliedObjects++;
	}

	return nResult;
}


//Returns Selected Object (Index at the Objects Array) and fill its Data to a structure
//Parameter: pObj - Structure Pointer to Fill Object Data
//Return Value: Selected Object Index
RENDERVIEW_API int  RENDERVIEW_CC GetSelectedObject(LPOUT_OBJECTINFO pObj)
{
	int nSel = -1;
	ObjectNode*	pNode;

	//Get Selected Object Index
	for (nSel = 0; nSel < g_arrObjects.GetCount(); nSel++)
	{
		if (g_arrObjects[nSel]->bPicked == true)
			break;
	}

	//Check if no object is Selected
	if (nSel >= g_arrObjects.GetCount())
		nSel = -1;

	//Fill Object Data of Selected
	if (pObj && (nSel >= 0))
	{
		pNode = g_arrObjects[nSel];

		pObj->nType = pNode->nType;
		_tcscpy(pObj->szId,	  pNode->szId);
		_tcscpy(pObj->szName, pNode->szName);
		_tcscpy(pObj->szCode, pNode->szCode);
		_tcscpy(pObj->szModel,pNode->szModel);

		pObj->fPosition[0]	= pNode->x;
		pObj->fPosition[1]	= pNode->y;
		pObj->fPosition[2]	= pNode->z;

		pObj->fBoxSize[0]	= pNode->w;
		pObj->fBoxSize[1]	= pNode->d;
		pObj->fBoxSize[2]	= pNode->h;

		pObj->fRotation[0]	= pNode->xa;
		pObj->fRotation[1]	= pNode->ya;
		pObj->fRotation[2]	= pNode->za;
	}

	return nSel;
}

//Returns Selected Object (Index at the Objects Array) and fill its Data to a structure
//Parameter: nObject - Object Index (at the Objects Array) to Get Parent Item
//		pObj - Structure Pointer to Fill Parent Object Data
//Return Value: Parent Object Index
RENDERVIEW_API int  RENDERVIEW_CC GetParentObject(int nObject, LPOUT_OBJECTINFO pObj)
{
	int nSel;
	ObjectNode*	pNode;
	if (nObject < 0 || nObject > g_arrObjects.GetCount())
		return -1;

	//Fill Object Data of Selected
	pNode	= g_arrObjects[nObject];
	pNode	= pNode->getParent();
	nSel	= g_arrObjects.GetIndex(&pNode, 0, FALSE);
	if (pObj && (nSel >= 0))
	{
		pObj->nType = pNode->nType;
		_tcscpy(pObj->szId,		pNode->szId);
		_tcscpy(pObj->szName,	pNode->szName);
		_tcscpy(pObj->szCode,	pNode->szCode);
		_tcscpy(pObj->szModel,	pNode->szModel);

		pObj->fPosition[0]	= pNode->x;
		pObj->fPosition[1]	= pNode->y;
		pObj->fPosition[2]	= pNode->z;

		pObj->fBoxSize[0]	= pNode->w;
		pObj->fBoxSize[1]	= pNode->d;
		pObj->fBoxSize[2]	= pNode->h;

		pObj->fRotation[0]	= pNode->xa;
		pObj->fRotation[1]	= pNode->ya;
		pObj->fRotation[2]	= pNode->za;
	}

	return nSel;
}


//Edit Object Data
//Parameter: nObject - Object Index (at the Objects Array) to Edit
//			pObj - Structure Pointer to New Object Data
//Return Value: Parent Object Index
RENDERVIEW_API int  RENDERVIEW_CC EditSelectedObject(int nObject, LPOUT_OBJECTINFO pObj)
{
	int nSel;
	ObjectNode*	pNode;
	if (nObject < 0 || nObject > g_arrObjects.GetCount())
		return 0;

	//Set New Data to Object
	pNode	= g_arrObjects[nObject];
	nSel	= g_arrObjects.GetIndex(&pNode, 0, FALSE);
	if (pObj && (nSel >= 0))
	{
		pNode->nType = pObj->nType;
		_tcscpy(pNode->szId,	pObj->szId);
		_tcscpy(pNode->szName,	pObj->szName);
		_tcscpy(pNode->szCode,	pObj->szCode);
		_tcscpy(pNode->szModel,	pObj->szModel);

		pNode->x	= pObj->fPosition[0];
		pNode->y	= pObj->fPosition[1];
		pNode->z	= pObj->fPosition[2];

		pNode->w	= pObj->fBoxSize[0];
		pNode->d	= pObj->fBoxSize[1];
		pNode->h	= pObj->fBoxSize[2];

		pNode->xa	= pObj->fRotation[0];
		pNode->ya	= pObj->fRotation[1];
		pNode->za	= pObj->fRotation[2];
	} else
		return 0;

	return 1;
}

TCHAR* RENDERVIEW_CC GetGroupNextInfo(TCHAR** ppNames, int nCount)
{
	int			nGroup, nObjects;	//Group Index, Object Count
	int			k, nLen;			//Loop Variables, Length
	TCHAR		*pResult, szTemp[XML_MAX_NAME_LEN];
	ObjectNode	**ppNodes, *pChild;

	ppNodes	= g_arrObjects.GetItem(0);
	nObjects= g_arrObjects.GetCount();
	for (nGroup = 0; nGroup < nObjects; nGroup++)
	{
		if (ppNodes[nGroup]->nType == XML_OT_GROUP)
			break;
	}

	if (nGroup >= nObjects)
		return NULL;

	//Get String Length
	nLen = 0;
	pChild = ppNodes[nGroup]->getChild();
	while (pChild)
	{
		for (k = 0; k < nCount; k++)
		{
			szTemp[0] = 0; //Empty Temp String
			switch (Parse_GetAttributeType(ppNames[k]))
			{
			case XML_ATTR_TYPE_ID:
				nLen += _tcslen(pChild->szId);
				break;

			case XML_ATTR_TYPE_NAME:
				nLen += _tcslen(pChild->szName);
				break;

			case XML_ATTR_TYPE_CODE:
				nLen += _tcslen(pChild->szCode);
				break;

			case XML_ATTR_TYPE_MODEL:
				nLen += _tcslen(pChild->szModel);
				break;

			case XML_ATTR_TYPE_ISMOVE:
				_tcscpy(szTemp, (pChild->isMove) ? _T("true") : _T("false"));
				break;
			case XML_ATTR_TYPE_ISDELETE:
				_tcscpy(szTemp, (pChild->isDelete) ? _T("true") : _T("false"));
				break;

			case XML_ATTR_TYPE_XPOS:
				_stprintf(szTemp, _T("%f"), pChild->vOrg.x);
				break;
			case XML_ATTR_TYPE_YPOS:
				_stprintf(szTemp, _T("%f"), pChild->vOrg.y);
				break;
			case XML_ATTR_TYPE_ZPOS:
				_stprintf(szTemp, _T("%f"), pChild->vOrg.z);
				break;

			case XML_ATTR_TYPE_WIDTH:
				_stprintf(szTemp, _T("%f"), pChild->w);
				break;
			case XML_ATTR_TYPE_DEPTH:
				_stprintf(szTemp, _T("%f"), pChild->d);
				break;
			case XML_ATTR_TYPE_HEIGHT:
				_stprintf(szTemp, _T("%f"), pChild->h);
				break;

			case XML_ATTR_TYPE_XROT:
				_stprintf(szTemp, _T("%f"), pChild->xa);
				break;
			case XML_ATTR_TYPE_YROT:
				_stprintf(szTemp, _T("%f"), pChild->ya);
				break;
			case XML_ATTR_TYPE_ZROT:
				_stprintf(szTemp, _T("%f"), pChild->za);
				break;

			case XML_ATTR_TYPE_PUTMODE:
				_tcscpy(szTemp, g_szPutModes[pChild->putMode]);
				break;

			case XML_ATTR_TYPE_MATERIAL:
				if (pChild->nMatIndex >= 0)
					nLen += _tcslen(g_arrMats[pChild->nMatIndex].szImagePath);
				break;

			case XML_ATTR_TYPE_ROTATE_MATERIAL:
				_tcscpy(szTemp, (pChild->rotateMaterial) ? _T("true") : _T("false"));
				break;
			}
			if (szTemp[0] != 0)
				nLen += _tcslen(szTemp);
			nLen += 2;	//Attribute Separator("||") length
		}
		nLen++;	//Item Separator('\n') Length
		//Process Next Sibling
		pChild = pChild->getSibling();
	}

	//alloc memory
	pResult = (TCHAR*) calloc(sizeof(TCHAR), (nLen + 15) & 0xFFFFFFF0);
	if (pResult == NULL)
		return NULL;

	//Get String Length
	nLen	= 0;
	pChild	= ppNodes[nGroup]->getChild();
	while (pChild)
	{
		for (k = 0; k < nCount; k++)
		{
			szTemp[0] = 0; //Empty Temp String
			switch (Parse_GetAttributeType(ppNames[k]))
			{
			case XML_ATTR_TYPE_ID:
				nLen += _stprintf(pResult + nLen, _T("%s||"), pChild->szId);
				break;

			case XML_ATTR_TYPE_NAME:
				nLen += _stprintf(pResult + nLen, _T("%s||"), pChild->szName);
				break;

			case XML_ATTR_TYPE_CODE:
				nLen += _stprintf(pResult + nLen, _T("%s||"), pChild->szCode);
				break;

			case XML_ATTR_TYPE_MODEL:
				nLen += _stprintf(pResult + nLen, _T("%s||"), pChild->szModel);
				break;

			case XML_ATTR_TYPE_ISMOVE:
				_tcscpy(szTemp, (pChild->isMove) ? _T("true") : _T("false"));
				break;
			case XML_ATTR_TYPE_ISDELETE:
				_tcscpy(szTemp, (pChild->isDelete) ? _T("true") : _T("false"));
				break;

			case XML_ATTR_TYPE_XPOS:
				_stprintf(szTemp, _T("%f"), pChild->x);
				break;
			case XML_ATTR_TYPE_YPOS:
				_stprintf(szTemp, _T("%f"), pChild->y);
				break;
			case XML_ATTR_TYPE_ZPOS:
				_stprintf(szTemp, _T("%f"), pChild->z);
				break;

			case XML_ATTR_TYPE_WIDTH:
				_stprintf(szTemp, _T("%f"), pChild->w);
				break;
			case XML_ATTR_TYPE_DEPTH:
				_stprintf(szTemp, _T("%f"), pChild->d);
				break;
			case XML_ATTR_TYPE_HEIGHT:
				_stprintf(szTemp, _T("%f"), pChild->h);
				break;

			case XML_ATTR_TYPE_XROT:
				_stprintf(szTemp, _T("%f"), D3DXToDegree(pChild->xa));
				break;
			case XML_ATTR_TYPE_YROT:
				_stprintf(szTemp, _T("%f"), D3DXToDegree(pChild->ya));
				break;
			case XML_ATTR_TYPE_ZROT:
				_stprintf(szTemp, _T("%f"), D3DXToDegree(pChild->za));
				break;

			case XML_ATTR_TYPE_PUTMODE:
				_tcscpy(szTemp, g_szPutModes[pChild->putMode]);
				break;

			case XML_ATTR_TYPE_MATERIAL:
				if (pChild->nMatIndex >= 0)
					nLen += _tcslen(g_arrMats[pChild->nMatIndex].szImagePath);
				break;

			case XML_ATTR_TYPE_ROTATE_MATERIAL:
				_tcscpy(szTemp, (pChild->rotateMaterial) ? _T("true") : _T("false"));
				break;
			}
			if (szTemp[0] != 0)
				nLen += _stprintf(pResult + nLen, _T("%s||"), szTemp);
		}
		//Remove Last Separator(||)
		nLen -= 2;
		//Add List Item Separator ('\n')
		pResult[nLen] = '\n';
		nLen++;
		//End String
		pResult[nLen] = 0;
		//Process Next Sibling
		pChild = pChild->getSibling();
	}

	//Remove Last Item Separator('\n')
	nLen--;
	pResult[nLen] = 0;

	return pResult;
}

void RENDERVIEW_CC ReleaseString(void* pString)
{
	if (pString != NULL)
		free(pString);
}
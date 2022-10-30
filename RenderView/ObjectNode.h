#pragma once
#ifndef __HOMEDECK_OBJECTNODE_HEADER__
#define __HOMEDECK_OBJECTNODE_HEADER__


#define		MAX_OBJECT_NAME_LEN		40

class ObjectNode
{
public:
	ObjectNode(void);

	~ObjectNode();

	void setParent(ObjectNode* pParent)		{ m_pParent = pParent; }
	void setSibling(ObjectNode* pSibling)	{ m_pSibling = pSibling; }
	void setChild(ObjectNode* pChild)		{ m_pFirstChild = pChild; }

	ObjectNode*	getParent()	 { return m_pParent; }
	ObjectNode* getSibling() { return m_pSibling; }
	ObjectNode* getChild()	 { return m_pFirstChild; }
	void setAloneSelected(bool bSelected) {m_bSelected = bSelected;};
	void setSelected(bool bSelected);
	bool isSelected() {return m_bSelected;};
	void setXmlAttributes(void* pDoc, void* pElem);

	bool setAloneColor();	//Set Color of Vertexes, not apply to child

	//Init Members
	void InitMembers();

	//Free Members
	void FreeMembers();

public:
	int		nType;		//OBJECT Type (Chest, Plane ...)

	//Values Get From File
	TCHAR	szId[MAX_OBJECT_NAME_LEN];
	TCHAR	szName[MAX_OBJECT_NAME_LEN];
	TCHAR	szCode[MAX_OBJECT_NAME_LEN];
	TCHAR	szModel[MAX_OBJECT_NAME_LEN];
	float	x, y, z;	//Position to be used in Rendering
	D3DXVECTOR3	vOrg;	//Position from File
	float	w, d, h;	//Size
	float	xa, ya, za;	//Rotation Angle in x, y, z
	float	cx,cy,cz;   //the relative coordinates of the center of the object to the global center
	float   fDecomp;
	BYTE	isMove;
	BYTE	isDelete;
	BYTE	rotateMaterial;	//Material Rotate Flag
	BYTE	bReserved;
	int		putMode;		//'putMode'
	int		intVtCount;
	int		intFaceCount;
	int     intLineVtCount;
	int     intLineCount;
	//Material
	int		nMatIndex;		//Index of Material Array, -1: no material

	//Rendering Values
	D3DXMATRIX	matWorld;
	D3DXMATRIX	matRotation;
	IDirect3DVertexBuffer9* pVertexBuffer;
	IDirect3DIndexBuffer9*	pIndexBuffer;
	IDirect3DVertexBuffer9* pLineVertexBuffer;
	IDirect3DIndexBuffer9*	pLineIndexBuffer;
	IDirect3DVertexBuffer9* pOutlineVertexBuffer;
	IDirect3DIndexBuffer9*	pOutLineIndexBuffer;
	D3DXVECTOR3	vBounds[6][4];
	bool		bPicked;
private:
	//Parent Node
	ObjectNode*	m_pParent;
	
	//Sibling Node
	ObjectNode*	m_pSibling;

	//Children Node
	ObjectNode* m_pFirstChild;

	//Selected State
	bool m_bSelected;
};

#endif	//__HOMEDECK_OBJECTNODE_HEADER__
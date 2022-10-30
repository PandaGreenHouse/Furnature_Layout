#pragma once
#include "MyArray.h"
#ifndef ___MY_3D_STRUCTURE_HEADER___
#define ___MY_3D_STRUCTURE_HEADER___

typedef struct tagLightSource
{
	int nType;//0-pointlight, 1-spotlight, 2-directional light
	D3DXVECTOR3 vPos;
	float fPower;
	float fAttenuation;
	float fMaxDist;
}LightSource;

typedef struct tagRay
{
	D3DXVECTOR3 vRayPos;
	D3DXVECTOR3 vDir;
	D3DXVECTOR3 vEntryPos;
	D3DXVECTOR3 vIntersection;
	D3DXVECTOR3 vxs[4], vNormals[4];
	D3DXVECTOR2 txes[3];//문양자리표
	BOOL        bLine;//이전광선과 현재광선이 한 직선상에 놓이면 1, 아니면 0
	BOOL		bHit;
	int			nObjectId;//이전의 광선과 사귄 오브젝트
	int			nFaceIndex;//이전의 광선과 사귄 삼각형요소 
	int			nCount;//추적회수
	float		u,v,dist;//사귐점의 파라메터
	float		fIntensities[2];//그로우명암처리용, 이전의 광선처리에서 구한 량
	int			nType;//0-Lambert, 1-Gouraud, 2-Phong
}Ray;

typedef struct tagFaceInfo
{
	int intFaceId;
	int intObjectId;
}FaceInfo;

typedef struct tagBSPTreeNode
{
	int nType;
	D3DXVECTOR3 vMin, vMax;
	D3DXVECTOR3 vxes[8];
	int intFaceCount;
	FaceInfo* pFaces;
	tagBSPTreeNode* pChilds[2];
}BSPTreeNode;

typedef struct tagGlobalMODEL
{
	int	 VertexCount; 
	int  FaceCount;
	int  SubModelCount;
	int  MaterialCount;
}GlobalModel,*LPGlobalModel;

typedef struct tagSubModel
{
	int StartVertex;
	int VertexCount;
	int StartFace;
	int FaceCount;
	TCHAR MaterialName[32];
}SubModel,*LPSubModel;

typedef struct tagMaterial
{
	DWORD MatAmbiemt;
	DWORD MatDiffuse;
	DWORD MatSpecular;
	TCHAR ImagePath[128];
	float Offset_U;
	float Offset_V;
	float ScaleU;
	float ScaleV;
	float TileU;
	float TileV;
	LPDIRECT3DTEXTURE9	pTexture;
}Material,*LPMaterial;

typedef struct tagVERTEX
{
	D3DXVECTOR3 Point;
	D3DXVECTOR3 Normal;
	DWORD	Diffuse;
	DWORD	Specular;
	float		u,v;
}VERTEX,*LPVERTEX;//D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2
typedef struct tagPlaneModel
{
	D3DXVECTOR3 points[4];
	int vxIndices[6];
	int lineIndices[8];
	int normalId;
	int matId;
	bool bPicked;
	int property;//0:inner plane, 1:outer plane
	IDirect3DVertexBuffer9* pVertexBuffer;
	IDirect3DIndexBuffer9* pIndexBuffer;
}PlaneModel,*LPPlaneModel;
typedef struct tagPlank
{
	float Width, Height, Depth;
	D3DXVECTOR3 vPos;
	D3DXVECTOR3 points[8];
	PlaneModel	PlaneModels[6];
	int         matId;
	bool        ismove;
	bool        isdelete;
	bool		bPicked;
	TCHAR		ImagePath;
	LPDIRECT3DTEXTURE9 pTexture;
	int			nXmlObjId;	//Index of XML Object Array
	float xa,ya,za;
}Plank;

typedef struct tagChestModel
{
	float width, height,depth;
	float x,y,z,xa,ya,za;
	Plank* planks;
	int plankCount;
	int nIntersectCount;
	D3DXVECTOR3 *pIntersects;
	D3DXVECTOR3 vCenter;
	D3DXVECTOR3 vOrigin;
}ChestModel;

struct TexCoord
{
	float tu, tv;
};
typedef struct tagLineVertex
{
	D3DXVECTOR3 vertex;
	DWORD diffuseColor;
}LineVertex;
typedef struct tagCamera
{
	D3DXVECTOR3 vCenter;
	D3DXVECTOR3 vEyePos;
	D3DXVECTOR3 vLookAt;
	D3DXVECTOR3 vObjectPos;
	D3DXVECTOR3 vUp;
	int mode;//viewmode 0:PERSPECTIVE 1:ORTHOGRAPHIC
	float		width, height, length;
	float fMax;
	float farDist;
	float nearDist;
	float fDist;//the nearest distance to the object
	float fov;
	float aspect;
}Camera;
enum type {CHEST, PLANK, GROOVE, HOLE, MODEL};
typedef struct Model_Node
{
	type    nType;//chest, plank, groove, hole ...
	float	x, y, z;	//Position
	float	w, d, h;	//Size
	float	xa, ya, za;	//Rotation Angle in x, y, z
	BYTE	isMove;
	BYTE	isDelete;
	BYTE	rotateMaterial;	//Material Rotate Flag
	BYTE	bReserved;
	int		putMode;		//'putMode'
	D3DXVECTOR3	d3dVertices[8];
	//Material
	int		nMatIndex;		//Index of Material Array, -1: no material

	int			intVertexCount;
	int			intFaceCount;
	int			intChildCount;
	IDirect3DVertexBuffer9* pVertexBuffer;
	IDirect3DIndexBuffer9* pIndexBuffer;
	Model_Node* pParent;
	Model_Node* pSibling;
	Model_Node* pFirstChild;
	int nStartObj;
}Model_Node,*LPModel_Node;
typedef struct tagModelInfo
{
	int intObjectCount;
	int intMatCount;
} ModelInfo;
typedef struct LineNode
{
	D3DXVECTOR3* pVertices;
	int nCount;
	LineNode* next;
} LineNode;
typedef struct tagTransformation
{
	D3DXVECTOR3 vTranslation;
	D3DXVECTOR3 vSpinPos;
	D3DXQUATERNION QRotation;
	D3DXVECTOR3 vScaleCenter;
	D3DXVECTOR3 vScaling;
}Transformation;
typedef struct tagTreeNode
{
	int intId;
	int intParent;
	int nStart;
	int nCount;
	D3DXVECTOR3 vPos;
	float xa,ya,za;
	D3DXMATRIX matWorld;
	D3DXMATRIX matRot;
}TreeNode;
extern LineNode g_LineNode;
extern Camera g_MyCameras[4];//0:top, 1:front, 2:side, 3:perspective
#define D3DFVF_MY_VERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)
#define D3DFVF_GEOMETRY (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))
#define SCROLL_DELTA	120.0f
#define GLOBAL_COLOR	D3DCOLOR_COLORVALUE(0.2,0.5,0.6,1.0)
#define PICK_COLOR		D3DCOLOR_COLORVALUE(1.0,0.0,0.0,1.0)
#define LINE_VERTEX		( D3DFVF_XYZ | D3DFVF_DIFFUSE )
#define  FULL_MODE 0
#define  WIRE_MODE 1
#define  SHAPE_MODE 2

#define VIEW_ALL			0
#define VIEW_TOP			1
#define VIEW_FRONT			2
#define VIEW_LEFT			3
#define VIEW_PERSPECTIVE	4

#define PERSPECTIVE	0
#define ORTHOGRAPHY 1

//-------------------------------Structures and Constants of XML File Data-------------------------------------------//

//String Length Constants
#define XML_MAX_NAME_LEN	40
#define XML_MAX_PATH_LEN	512

//Object Types
#define XML_OT_MAX_COUNT	8	//Object Type Count
#define XML_OT_UNKNOWN		0	//Unknown Object Type
#define XML_OT_GROUP		1	//'group'	Root Object
#define XML_OT_CHEST		2	//'chest'
#define XML_OT_MODULE		3	//'module'
#define XML_OT_PLANE		4	//'plank'
#define XML_OT_HOLE			5	//'hole'
#define XML_OT_HARDWARE		6	//'hardware'
#define XML_OT_GROOVE		7	//'groove'

//putMode Constants
#define XML_PM_MAX_COUNT	3	//PutMode Count
#define XML_PM_UNKNOWN		-1	//Unknown PutMode
#define XML_PM_HORIZONTAL	0	//横放 = 0,
#define XML_PM_VERTICAL		1	//竖放 = 1,
#define XML_PM_CUBIC		2	//平放 = 2,'

//Material Type Constants
#define	XML_MT_COLOR		0
#define	XML_MT_IMAGE		1

typedef struct tagXML_MATINFO
{
	int		nType;
	int		nAppliedObjects;

	float	fAmbientColor[4];
	float	fDiffuseColor[4];
	float	fSpecularColor[4];

	TCHAR	szImagePath[XML_MAX_PATH_LEN];
	LPDIRECT3DTEXTURE9 pTexture;
	int width, height;
	D3DXCOLOR* pColors;
}XML_MATINFO, *LPXML_MATINFO;

typedef struct tagXML_OBJECT
{
	int		nType;		//OBJECT Type (Chest, Plane ...)

	//Values Get From File
	TCHAR	szId[XML_MAX_NAME_LEN];
	TCHAR	szName[XML_MAX_NAME_LEN];
	TCHAR	szCode[XML_MAX_NAME_LEN];
	TCHAR	szModel[XML_MAX_NAME_LEN];
	float	x, y, z;	//Position
	float	w, d, h;	//Size
	float	xa, ya, za;	//Rotation Angle in x, y, z
	BYTE	isMove;
	BYTE	isDelete;
	BYTE	rotateMaterial;	//Material Rotate Flag
	BYTE	bReserved;
	int		putMode;		//'putMode'

	//Material
	int		nMatIndex;		//Index of Material Array, -1: no material

	//Children Relation
	int		nStartChild;	//Children Start Index of Array
	int		nChildCount;	//Children Count of this object
}XML_OBJECT, *LPXML_OBJECT;


//-------------------------------Structures and Constants of XML File Data-------------------------------------------//

typedef struct tagModelObject
{
	XML_OBJECT xmlData;
	int intId;
	bool bPicked;
	D3DXMATRIX worldMat;
}ModelObject;


//-----------------Structures for WPF Communication--------------------------//

typedef struct tagOUT_OBJECTINFO
{
	int		nType;		//OBJECT Type (Chest, Plane ...)

	//Values Get From File
	TCHAR	szId[XML_MAX_NAME_LEN];
	TCHAR	szName[XML_MAX_NAME_LEN];
	TCHAR	szCode[XML_MAX_NAME_LEN];
	TCHAR	szModel[XML_MAX_NAME_LEN];

	float	fPosition[3];	//Position
	float	fBoxSize[3];	//Size
	float	fRotation[3];	//Rotation Angle in x, y, z
}OUT_OBJECTINFO, *LPOUT_OBJECTINFO;

//---Material Info
typedef struct tagOUT_MATINFO
{
	int		nType;

	float	fAmbientColor[4];
	float	fDiffuseColor[4];
	float	fSpecularColor[4];

	TCHAR	szImagePath[XML_MAX_PATH_LEN];
}OUT_MATINFO, *LPOUT_MATINFO;

//-----------------Structures for WPF Communication--------------------------//

#endif	//___MY_3D_STRUCTURES_HEADER___
#include "stdafx.h"
#include "ModelObject.h"
#include "PlankModel.h"
#include "Model.h"
#include "CylinderModel.h"
#include "RenderView.h"
#include "BSPTree.h"
CBSPTree g_ClsTree;

TCHAR* _copy_char2tchar(TCHAR* dst, char* src);
char* _copy_tchar2char(char* dst, TCHAR* src);

CModelObject::CModelObject(void)
{
	m_ppTextures = NULL;
	m_intRootCount = 0;
	m_ArrRootNodes.Clear();
	m_ArrRootNodes.Create(10,10);
}

void CModelObject::CreateModelObject(LPDIRECT3DDEVICE9 pd3dDevice)
{
	CreateMaterials(pd3dDevice);
	CPlankModel plank;
	CCylinderModel   cylinder;
	int nObjects = g_arrObjects.GetCount();
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);

	for(int i=0; i < nObjects; i++)
	{
		ppNodes[i]->bPicked = false;
		plank.ComputingBoundingBox(ppNodes[i]);/*compute each object's bounding box in plank class*/
		switch(ppNodes[i]->nType)
		{
		case XML_OT_PLANE:
			plank.CreatePlankModel(pd3dDevice, ppNodes[i]);	
			//TransformObject(ppNodes[i]);
			break;
		case XML_OT_HOLE:
			cylinder.CreateModel(pd3dDevice, ppNodes[i], 20);
			//TransformObject(ppNodes[i]);
			break;
		case XML_OT_GROOVE:
			plank.CreatePlankModel(pd3dDevice, ppNodes[i]);	
			//TransformObject(ppNodes[i]);
			break;
		}
		TransformObject(ppNodes[i]);
		ComputingNormal(ppNodes[i]);
	}
	ComputingRelativeCenter();
	g_ClsTree.CreateTree();
}

bool CModelObject::CreateMaterials(LPDIRECT3DDEVICE9 pd3dDevice)
{
	LPXML_MATINFO	pMatArray = g_arrMats.GetItem(0);// pointer of material array
	int			    nMatCount = g_arrMats.GetCount();
	TCHAR			szPath[XML_MAX_PATH_LEN];
	for(int i=0; i<nMatCount; i++)
	{
		for(int j=0; j<3; j++)
			pMatArray[i].fDiffuseColor[j] = 0.5f;
		pMatArray[i].pTexture = NULL;
		if (pMatArray[i].szImagePath[0] == 0) continue;
		szPath[0] = 0;
		GetImageAbsolutePath(szPath, pMatArray[i].szImagePath);
		pMatArray[i].pTexture = NULL;
		if (CheckFileExists(szPath))
		{
			if (D3DXCreateTextureFromFile( pd3dDevice, szPath, &(pMatArray[i].pTexture) )!=D3D_OK)
				return false;
			char fileName[MAXCHAR];
			_copy_tchar2char(fileName, szPath);
			IplImage* lpImg = cvLoadImage(fileName);
			pMatArray[i].pColors = new D3DXCOLOR[lpImg->width*lpImg->height];
			pMatArray[i].width = lpImg->width;
			pMatArray[i].height = lpImg->height;
			CvScalar s;
			int l=0;
			for(int ix=0; ix<lpImg->height; ix++)
			{
				for(int iy=0; iy<lpImg->width; iy++)
				{
					s = cvGet2D(lpImg,ix,iy);
					pMatArray[i].pColors[l].b = s.val[0]/255;
					pMatArray[i].pColors[l].g = s.val[1]/255;
					pMatArray[i].pColors[l].r = s.val[2]/255;
					l++;
				}
			}
		}
	}
	return true;
}

/*apply world matrices and rotation matrices to the objects to transform model. note that rotation transformation must be 
applied to the normals of the vertexes */
void CModelObject::TransformObject(ObjectNode* pNode)//D3DXMATRIX* matWorld, D3DXMATRIX* rotMat, IDirect3DVertexBuffer9* pVertexBuffer )
{
	/*transform the center of object*/
	D3DXVECTOR4 vec4;
	D3DXVECTOR3 vCenter;
	vCenter.x = pNode->x;
	vCenter.y = pNode->y;
	vCenter.z = pNode->z;
	D3DXVec3Transform(&vec4,&vCenter,&pNode->matWorld);
	pNode->x = vec4.x;
	pNode->y = vec4.y;
	pNode->z = vec4.z;
	/*transform the bounding box*/
	for(int i=0; i<6; i++) 
	{
		for(int j=0; j<4; j++)
		{
			D3DXVec3Transform(&vec4, &(pNode->vBounds[i][j]), &pNode->matWorld);
			pNode->vBounds[i][j].x = vec4.x;
			pNode->vBounds[i][j].y = vec4.y;
			pNode->vBounds[i][j].z = vec4.z;
		}
	}
	/*transform the vertices of the model object*/
	if(pNode->pVertexBuffer==NULL) return;
	D3DVERTEXBUFFER_DESC desc;
	pNode->pVertexBuffer->GetDesc(&desc);
	int nSize = desc.Size;
	int nCount = nSize/sizeof(VERTEX);
	byte* pBytes;
	if(pNode->pVertexBuffer->Lock(0,0,(void**)&pBytes,D3DLOCK_DISCARD)!=D3D_OK)
		return;
	D3DXMATRIX matWorld, rotMat;
	matWorld = pNode->matWorld;
	rotMat = pNode->matRotation;
	VERTEX* pVertices = (VERTEX*)pBytes;
	for(int j=0; j<nCount; j++)
	{
		D3DXVec3Transform(&vec4, &pVertices[j].Point, &matWorld);
		pVertices[j].Point.x = vec4.x;
		pVertices[j].Point.y = vec4.y;
		pVertices[j].Point.z = vec4.z;
		/*D3DXVec3Transform(&vec4, &pVertices[j].Normal, &rotMat);
		pVertices[j].Normal.x = vec4.x;
		pVertices[j].Normal.y = vec4.y;
		pVertices[j].Normal.z = vec4.z;*/
	}
	pNode->pVertexBuffer->Unlock();
	/*transform the vertices of the lines*/
	LineVertex* pLineVertices;
	if(pNode->pLineVertexBuffer==NULL) return;
	if(pNode->pLineVertexBuffer->Lock(0,0,(void**)&pLineVertices,D3DLOCK_DISCARD)!=D3D_OK)
		return;
	for(int j=0; j<pNode->intLineVtCount; j++)
	{
		D3DXVec3Transform(&vec4, &pLineVertices[j].vertex, &matWorld);
		pLineVertices[j].vertex.x = vec4.x;
		pLineVertices[j].vertex.y = vec4.y;
		pLineVertices[j].vertex.z = vec4.z;
	}
	pNode->pLineVertexBuffer->Unlock();
}

void CModelObject::ComputingNormal(ObjectNode* pNode)
{
	if(pNode->pVertexBuffer==NULL) return;
	byte* pBytes;
	if(pNode->pVertexBuffer->Lock(0,0,(void**)&pBytes,D3DLOCK_DISCARD)!=D3D_OK)
		return;
	VERTEX* pVertices = (VERTEX*)pBytes;
	/*D3DXVECTOR3 vCenter = pVertices[0].Point;
	for(int i=1; i<pNode->intVtCount-1; i++)
	{
		vCenter = ((float) i*vCenter + pVertices[i+1].Point)/(float)(i+1);
	}
	D3DXVECTOR3 vNormal;
	for(int i=0; i<pNode->intVtCount; i++)
	{
		vNormal = pVertices[i].Point - vCenter;
		D3DXVec3Normalize(&vNormal,&vNormal);
		pVertices[i].Normal = vNormal;
	}*/
	
	if(pNode->pIndexBuffer->Lock(0,0,(void**)&pBytes,D3DLOCK_DISCARD)!=D3D_OK)
		return;
	int* pIndices = (int*)pBytes;
	D3DXVECTOR3 vec[3];
	for(int i=0; i<pNode->intFaceCount; i++)
	{
		int t[3];
		t[0] = pIndices[3*i];
		t[1] = pIndices[3*i+1];
		t[2] = pIndices[3*i+2];
		vec[0] = pVertices[t[1]].Point - pVertices[t[0]].Point;
		vec[1] = pVertices[t[2]].Point - pVertices[t[0]].Point;
		D3DXVec3Cross(&vec[2],&vec[1],&vec[0]);
		D3DXVec3Normalize(&vec[2],&vec[2]);
		pVertices[t[0]].Normal = vec[2];
		pVertices[t[1]].Normal = vec[2];
		pVertices[t[2]].Normal = vec[2];
	}
	/*byte* pFlags = (byte*)calloc(pNode->intVtCount,1); 
	int count = 0;
	D3DXVECTOR3 vNormal;
	for(int i=0; i<pNode->intVtCount; i++)
	{
		if(pFlags[i]==1) continue;
		pFlags[i] = 1;
		count = 0;
		vNormal = D3DXVECTOR3(0.0f,0.0f,0.0f);
		for(int j=0; j<pNode->intVtCount; j++)
		{
			if(i==j) continue;
			if(pVertices[i].Point == pVertices[j].Point )
			{
				vNormal += pVertices[i].Normal;
			}
			pFlags[j] = 1;
			count++;
		}
		if(count==0) continue;
		D3DXVec3Normalize(&vNormal,&vNormal);
		pVertices[i].Normal = vNormal;
		for(int j=0; j<pNode->intVtCount; j++)
		{
			if(pVertices[i].Point==pVertices[j].Point)
				pVertices[j].Normal = vNormal;
		}
	}
	free(pFlags);*/
	pNode->pVertexBuffer->Unlock();
	pNode->pIndexBuffer->Unlock();
}

void CModelObject::ComputingRelativeCenter()
{
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	/*get global center*/
	ppNodes[0]->cx = ppNodes[0]->x + ppNodes[0]->w/2;
	ppNodes[0]->cy = ppNodes[0]->y + ppNodes[0]->d/2;
	ppNodes[0]->cz = ppNodes[0]->z + ppNodes[0]->h/2;
	ppNodes[0]->fDecomp = 1.0f;
	for(int i=1; i<g_arrObjects.GetCount(); i++)
	{
		ppNodes[i]->cx = ppNodes[i]->x + ppNodes[i]->w/2;
		ppNodes[i]->cy = ppNodes[i]->y + ppNodes[i]->d/2;
		ppNodes[i]->cz = ppNodes[i]->z + ppNodes[i]->h/2;
		ppNodes[i]->cx = ppNodes[i]->cx - ppNodes[0]->cx;
		ppNodes[i]->cy = ppNodes[i]->cy - ppNodes[0]->cy;
		ppNodes[i]->cz = ppNodes[i]->cz - ppNodes[0]->cz;
		ppNodes[i]->fDecomp = 1.0f;
	}
}
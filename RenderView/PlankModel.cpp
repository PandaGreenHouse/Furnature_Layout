#include "stdafx.h"
#include "Model.h"
#include "PlankModel.h"

int _PlaneVtIndices[6][4] = {0,1,2,3,//front
	5,4,7,6,//back
	4,0,3,7,//0,3,7,4,//left
	1,5,6,2,//right
	3,2,6,7,//2,6,7,3,//top
	4,5,1,0//0,4,5,1,//bottom
};
int PlaneIndices[6] = {0,1,2, 2,3,0 };

int PlankLineIndices[24] = {0,1, 1,2, 2,3, 3,0, //front
							4,5, 5,6, 6,7, 7,4, //back
							0,4, 1,5, 2,6, 3,7
							};
int Indices[36] = {0,1,2, 2,3,0, //front
					4,5,6, 6,7,4, //back
					8,9,10, 10,11,8,//left
					12,13,14, 14,15,12,//right
					16,17,18, 18,19,16,//top
					20,21,22, 22,23,20//bottom
					};
int PlaneLineIndices[8] = {0,1, 1,2, 2,3, 3,0 };
D3DXVECTOR3 vNormals[6]={D3DXVECTOR3(0,1,0),//front
	D3DXVECTOR3(0,-1,0),//back
	D3DXVECTOR3(-1,0,0),//left
	D3DXVECTOR3(1,0,0),//right
	D3DXVECTOR3(0,0,1),//top
	D3DXVECTOR3(0,0,-1)//bottom
};
	
D3DXVECTOR2 texCoordes[4] = {D3DXVECTOR2(0,0),D3DXVECTOR2(1,0),
	D3DXVECTOR2(1,1),D3DXVECTOR2(0,1),};

CPlankModel::CPlankModel(void)
{
}

void CPlankModel::CalculateVertices(float width, float depth, float height)
{
	D3DXVECTOR3 vOrigin = D3DXVECTOR3(0.0F,0.0F,0.0F);
	float Dx,Dy,Dz;
	Dx = width;
	Dy = depth;
	Dz = height;
	m_d3dVertices[0] = vOrigin;
	m_d3dVertices[1] = m_d3dVertices[0];
	m_d3dVertices[1].x += Dx; 
	m_d3dVertices[2] = m_d3dVertices[1];
	m_d3dVertices[2].z += Dz;
	m_d3dVertices[3] = m_d3dVertices[2];
	m_d3dVertices[3].x -= Dx;
	for(int i=0; i<4; i++)
	{
		m_d3dVertices[i+4] = m_d3dVertices[i];
		m_d3dVertices[i+4].y += Dy;
	}
}

void CPlankModel::CreatePlankModel(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode)
{
	CalculateVertices(pNode->w, pNode->d, pNode->h);
	XML_MATINFO* pMat = NULL;
	DWORD color = D3DCOLOR_COLORVALUE(1.0,1.0,1.0,1.0);
	int ix = pNode->nMatIndex;
	if (ix >= 0)
	{
		pMat = g_arrMats.GetItem(ix);
		if (pMat->nType==XML_MT_COLOR)
			color = D3DCOLOR_RGBA((DWORD) (pMat->fDiffuseColor[0]), (DWORD) (pMat->fDiffuseColor[1]), (DWORD) (pMat->fDiffuseColor[2]), 255);
	} //else	//Default Color
	//	color = D3DCOLOR_RGBA(128, 128, 128, 255);

	VERTEX vertices[6][4];
	for(int i=0; i<6; i++ )	
	{
		for(int j=0; j<4; j++)
		{
			vertices[i][j].Point = m_d3dVertices[_PlaneVtIndices[i][j]];
			vertices[i][j].Normal = vNormals[i];
			vertices[i][j].u = texCoordes[j].x;
			vertices[i][j].v = texCoordes[j].y;
			vertices[i][j].Diffuse = color;
			vertices[i][j].Specular = color;
			pNode->vBounds[i][j] = vertices[i][j].Point;
		}
	}

	int nSize = 24*sizeof(VERTEX);
	if(pd3dDevice->CreateVertexBuffer(nSize,0,D3DFVF_MY_VERTEX,D3DPOOL_DEFAULT,&(pNode->pVertexBuffer),NULL)!=D3D_OK) return;
	BYTE* pBytes = NULL;
	if( (pNode->pVertexBuffer)->Lock( 0, 0, (void**)&pBytes, D3DLOCK_DISCARD ) != D3D_OK ) return;
	memcpy( pBytes, vertices, nSize );
	(pNode->pVertexBuffer)->Unlock();

	nSize = 36*sizeof(int);
	if(pd3dDevice->CreateIndexBuffer(nSize, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED , &(pNode->pIndexBuffer), NULL)!=D3D_OK)	 return;
	if(pNode->pIndexBuffer->Lock(0, 0, (void**)&pBytes, D3DLOCK_DISCARD)!=D3D_OK)	 return; 
	memcpy( pBytes, Indices, nSize );
	pNode->pIndexBuffer->Unlock();
	pNode->intVtCount = 24;
	pNode->intFaceCount = 12;
	
	LineVertex LineVertices[8];
	for(int i=0; i<8; i++)
	{
		LineVertices[i].vertex = m_d3dVertices[i];
		LineVertices[i].diffuseColor = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,0.0);
	}
	nSize = 8*sizeof(LineVertex);
	if(pd3dDevice->CreateVertexBuffer(nSize,0,LINE_VERTEX,D3DPOOL_DEFAULT,&(pNode->pLineVertexBuffer),NULL)!=D3D_OK) return;
	pBytes = NULL;
	if( (pNode->pLineVertexBuffer)->Lock( 0, 0, (void**)&pBytes, D3DLOCK_DISCARD ) != D3D_OK ) return;
	memcpy( pBytes, LineVertices, nSize );
	(pNode->pLineVertexBuffer)->Unlock();
	pNode->intLineVtCount = 8;
	nSize = 24*sizeof(int);
	if(pd3dDevice->CreateIndexBuffer(nSize, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED , &(pNode->pLineIndexBuffer), NULL)!=D3D_OK)	 return;
	pBytes = NULL;
	if(pNode->pLineIndexBuffer->Lock(0, 0, (void**)&pBytes, D3DLOCK_DISCARD)!=D3D_OK)	 return; 
	memcpy( pBytes, PlankLineIndices, nSize );
	pNode->pLineIndexBuffer->Unlock();
	pNode->intLineCount = 12;
}

void CPlankModel::ComputingBoundingBox(ObjectNode* pNode)
{
	CalculateVertices(pNode->w, pNode->d, pNode->h);
	for(int i=0; i<6; i++)
	{
		for(int j=0; j<4; j++)
		{
			pNode->vBounds[i][j] = m_d3dVertices[_PlaneVtIndices[i][j]];
		}
	}
}

void CPlankModel::ComputingOutlineModel(ObjectNode* pNode)
{
	D3DXVECTOR3 vert3D[8][3];
	float delta = 5.0f;
	for(int i=0; i<8; i++)
	{
		vert3D[i][0] = m_d3dVertices[i];
	}
}
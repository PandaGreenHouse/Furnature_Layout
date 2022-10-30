#include "stdafx.h"
#include <math.h>
#include "CylinderModel.h"


CCylinderModel::CCylinderModel(void)
{
}

void CCylinderModel::CreateModel(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, int nSlices)
{
	int nVertices = 2*(nSlices+1) + 2;
	float fRadius = pNode->w;
	float fHeight = pNode->h;
	LPVERTEX pVertices = new VERTEX[nVertices];
	float seta = 2*D3DX_PI/nSlices;
	DWORD color = D3DCOLOR_COLORVALUE(0.0,1.0,0.0,1.0);
	for(int i=0; i<nSlices+1; i++)
	{
		/*bottom vertices*/
		pVertices[i].Point.x = fRadius*cos(i*seta);
		pVertices[i].Point.y = fRadius*sin(i*seta);
		pVertices[i].Point.z = 0.0f;
		pVertices[i].Diffuse = color;
		pVertices[i].Specular = color;
		/*top vertices*/
		pVertices[i + nSlices+1].Point.x = fRadius*cos(i*seta);
		pVertices[i + nSlices+1].Point.y = fRadius*sin(i*seta);
		pVertices[i + nSlices+1].Point.z = fHeight;
		pVertices[i + nSlices+1].Diffuse = color;
		pVertices[i + nSlices+1].Specular = color;
	}
	pVertices[nVertices-2].Point = D3DXVECTOR3(0.0f,0.0f,0.0f);//bottom center
	pVertices[nVertices-1].Point = D3DXVECTOR3(0.0f,0.0f,fHeight);//top center
	int nFaces = 4*nSlices;
	int* pIndices = new int[3*nFaces];
	/* bottom faces */
	for(int i=0; i<nSlices; i++)
	{
		pIndices[3*i] = i;
		pIndices[3*i+1] = nVertices-2;
		pIndices[3*i+2] = i+1;
	}
	/* top faces */
	int j = (nVertices-2)/2;
	for(int i=nSlices; i<2*nSlices; i++)
	{
		pIndices[3*i] = j;
		pIndices[3*i+1] = nVertices-1;
		pIndices[3*i+2] = j+1;
		j++;
	}
	/*surounding surface*/
	j=0;
	int k = (nVertices-2)/2;
	for(int i=2*nSlices; i<3*nSlices; i++)
	{
		pIndices[3*i] = j;
		pIndices[3*i+1] = j+1;
		pIndices[3*i+2] = k++;
		j++;
	}
	j=0;
	k = (nVertices-2)/2;
	for(int i=3*nSlices; i<4*nSlices; i++)
	{
		pIndices[3*i] = j;
		pIndices[3*i+1] = k+1;
		pIndices[3*i+2] = k;
		j++;
		k++;
	}
	/*creating vertexbuffer and indexbuffer*/
	int nSize = nVertices*sizeof(VERTEX);
	if(pd3dDevice->CreateVertexBuffer(nSize,0,D3DFVF_MY_VERTEX,D3DPOOL_DEFAULT,&(pNode->pVertexBuffer),NULL)!=D3D_OK) return;
	BYTE* pBytes = NULL;
	if( (pNode->pVertexBuffer)->Lock( 0, 0, (void**)&pBytes, D3DLOCK_DISCARD ) != D3D_OK ) return;
	memcpy( pBytes, pVertices, nSize );
	(pNode->pVertexBuffer)->Unlock();

	nSize = 3*nFaces*sizeof(int);
	if(pd3dDevice->CreateIndexBuffer(nSize, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED , &(pNode->pIndexBuffer), NULL)!=D3D_OK)	 return;
	if(pNode->pIndexBuffer->Lock(0, 0, (void**)&pBytes, D3DLOCK_DISCARD)!=D3D_OK)	 return; 
	memcpy( pBytes, pIndices, nSize );
	pNode->pIndexBuffer->Unlock();

	pNode->intVtCount = nVertices;
	pNode->intFaceCount = nFaces;
	
	/*Create lines*/
	pNode->intLineVtCount = 2*nSlices;
	LineVertex* pLineVertices = new LineVertex[pNode->intLineVtCount];
	color = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,1.0);
	for(int i=0; i<nSlices; i++)
	{
		pLineVertices[i].vertex.x = fRadius*cos(i*seta);
		pLineVertices[i].vertex.y = fRadius*sin(i*seta);
		pLineVertices[i].vertex.z = 0.0f;
		pLineVertices[i].diffuseColor = color;
	}
	for(int i=nSlices; i<2*nSlices; i++)
	{
		pLineVertices[i].vertex.x = fRadius*cos(i*seta);
		pLineVertices[i].vertex.y = fRadius*sin(i*seta);
		pLineVertices[i].vertex.z = fHeight;
		pLineVertices[i].diffuseColor = color;
	}
	nSize = pNode->intLineVtCount*sizeof(LineVertex);
	if(pd3dDevice->CreateVertexBuffer(nSize,0,LINE_VERTEX,D3DPOOL_DEFAULT,&(pNode->pLineVertexBuffer),NULL)!=D3D_OK) return;
	pBytes = NULL;
	if( (pNode->pLineVertexBuffer)->Lock( 0, 0, (void**)&pBytes, D3DLOCK_DISCARD ) != D3D_OK ) return;
	memcpy( pBytes, pLineVertices, nSize );
	(pNode->pLineVertexBuffer)->Unlock();
	
	pNode->intLineCount = 3*nSlices;
	int* pLineIndices = new int[2*pNode->intLineCount];
	memset(pLineIndices,0,2*pNode->intLineCount*sizeof(int));
	for(int i=0; i < nSlices-1; i++)
	{
		pLineIndices[2*i] = i;
		pLineIndices[2*i+1] = i+1;
	}
	pLineIndices[2*(nSlices-1)] = nSlices - 1;
	pLineIndices[2*(nSlices-1)+1] = 0;

	for(int i=nSlices; i < 2*nSlices-1; i++)
	{
		pLineIndices[2*i] = i;
		pLineIndices[2*i+1] = i+1;
	}
	pLineIndices[2*(2*nSlices-1)] = 2*nSlices - 1;
	pLineIndices[2*(2*nSlices-1)+1] = nSlices;

	for(int i=0; i<nSlices; i++)
	{
		pLineIndices[2*i + 2*nSlices] = i;
		pLineIndices[2*i + 1 + 2*nSlices] = nSlices + i;
	}
	nSize = 2*pNode->intLineCount*sizeof(int);
	if(pd3dDevice->CreateIndexBuffer(nSize, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED , &(pNode->pLineIndexBuffer), NULL)!=D3D_OK)	 return;
	if(pNode->pLineIndexBuffer->Lock(0, 0, (void**)&pBytes, D3DLOCK_DISCARD)!=D3D_OK)	 return; 
	memcpy( pBytes, pLineIndices, nSize );
	pNode->pLineIndexBuffer->Unlock();
}

#include "stdafx.h"
#include "BSPTree.h"
#include "MyArray.h"
#include "Model.h"
#define LIM_COUNT	35
#define LIM_SIZE	0.1f
#define EPSLEN		0.01f;

int g_intTotalCount;
int g_intLeafCount;

CBSPTree::CBSPTree(void)
{
	m_pRoot = NULL;
	g_intLeafCount = g_intTotalCount = 0;
}


CBSPTree::~CBSPTree(void)
{
	FreeTree();
}

BOOL CBSPTree::CheckRayIntersection(Ray* pRay)
{
	pRay->bHit = FALSE;
	pRay->vEntryPos = pRay->vRayPos;
	pRay->dist = FLT_MAX;
	CheckNode(m_pRoot, pRay);
	return pRay->bHit;
}

void CBSPTree::FreeTree()
{
	m_intLeafCount = m_intNodeCount = 0;
	if(m_pRoot==NULL) return;
	FreeNode(m_pRoot);
}

void CBSPTree::FreeNode(BSPTreeNode* pNode)
{
	if(pNode==NULL) return;
	FreeNode(pNode->pChilds[0]);
	FreeNode(pNode->pChilds[1]);
	if(pNode->pFaces){
		delete[] pNode->pFaces;
		pNode->pFaces = NULL;
	}
	delete pNode;
	pNode = NULL;
}

void CBSPTree::CreateTree()
{
	FreeTree();
	CreateRootNode();
	CreateChildren(m_pRoot);
}

float CBSPTree::GetLimitSize(BSPTreeNode* pNode)
{
	D3DXVECTOR3 vDim = pNode->vMax - pNode->vMin;
	FLOAT fMax = LIM_SIZE*max(vDim.x,max(vDim.y,vDim.z));
	return fMax;
}

void CBSPTree::GetBoundingBox(D3DXVECTOR3* vMin, D3DXVECTOR3* vMax)
{
	*vMin = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);
	*vMax = D3DXVECTOR3(FLT_MIN,FLT_MIN,FLT_MIN);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if(!g_arrObjects[i]->pVertexBuffer) continue;
		D3DVERTEXBUFFER_DESC desc;
		g_arrObjects[i]->pVertexBuffer->GetDesc(&desc);
		int nVertCount = desc.Size/sizeof(VERTEX);
		VERTEX* pVertices;
		if(g_arrObjects[i]->pVertexBuffer->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD)!=D3D_OK)
			continue;
		for(int j=0; j<nVertCount; j++)
		{
			//pVertices[j].Normal *= -1.0f;
			if(vMin->x > pVertices[j].Point.x)
				vMin->x = pVertices[j].Point.x;
			if(vMin->y > pVertices[j].Point.y)
				vMin->y = pVertices[j].Point.y;
			if(vMin->z > pVertices[j].Point.z)
				vMin->z = pVertices[j].Point.z;

			if(vMax->x < pVertices[j].Point.x)
				vMax->x = pVertices[j].Point.x;
			if(vMax->y < pVertices[j].Point.y)
				vMax->y = pVertices[j].Point.y;
			if(vMax->z < pVertices[j].Point.z)
				vMax->z = pVertices[j].Point.z;
		}
		g_arrObjects[i]->pVertexBuffer->Unlock();
	}
}

void CBSPTree::CreateRootNode()
{
	m_pRoot = new BSPTreeNode;
	CMyArray<FaceInfo*> arrFaces;
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if(g_arrObjects[i]->intFaceCount<0) continue;
		for(int j=0; j<g_arrObjects[i]->intFaceCount; j++)
		{
			FaceInfo* pFace = new FaceInfo;
			pFace->intFaceId = j;
			pFace->intObjectId = i;
			arrFaces.AddItem(&pFace,1);
		}
	}
	m_pRoot->intFaceCount = arrFaces.GetCount();
	m_pRoot->pFaces = new FaceInfo[m_pRoot->intFaceCount];
	for(int i=0; i<m_pRoot->intFaceCount; i++)
	{
		m_pRoot->pFaces[i].intFaceId = arrFaces[i]->intFaceId;
		m_pRoot->pFaces[i].intObjectId = arrFaces[i]->intObjectId;
	}
	arrFaces.Clear();
	GetBoundingBox(&m_pRoot->vMin,&m_pRoot->vMax);
	GetVerticesOfAABBOfNode(m_pRoot);
	m_fLimSize = LIM_SIZE*GetLimitSize(m_pRoot);
}

void CBSPTree::CreateChildren(BSPTreeNode* pNode)
{
	g_intTotalCount++;
	pNode->nType = 0;
	D3DXVECTOR3 vDim = pNode->vMax - pNode->vMin;
	float fMax = max(vDim.x,max(vDim.y,vDim.z));
	if(pNode->intFaceCount < LIM_COUNT || fMax < m_fLimSize){
		pNode->nType = 1;
		pNode->pChilds[0] = NULL;
		pNode->pChilds[1] = NULL;
		g_intLeafCount++;
		return;
	}
	
	int nFlag = 0;
	if(vDim.x==fMax) nFlag = 0;
	if(vDim.y==fMax) nFlag = 1;
	if(vDim.z==fMax) nFlag = 2;
	FaceInfo* pFaces[2];
	pFaces[0] = new FaceInfo[pNode->intFaceCount];
	pFaces[1] = new FaceInfo[pNode->intFaceCount];
	int nFaceCount[2];
	D3DXVECTOR3 vMin = pNode->vMin;
	switch(nFlag)
	{
	case 0://yz
		vDim.x *= 0.5f;
		vMin.x += vDim.x;
		PartitionYZ(pFaces[0], &nFaceCount[0], pFaces[1], &nFaceCount[1], pNode->pFaces, pNode->intFaceCount,vMin.x);
		break;
	case 1://zx
		vDim.y *= 0.5f;
		vMin.y += vDim.y;
		PartitionZX(pFaces[0], &nFaceCount[0], pFaces[1], &nFaceCount[1], pNode->pFaces, pNode->intFaceCount,vMin.y);
		break;
	case 2://xy
		vDim.z *= 0.5f;
		vMin.z += vDim.z;
		PartitionXY(pFaces[0], &nFaceCount[0], pFaces[1], &nFaceCount[1], pNode->pFaces, pNode->intFaceCount,vMin.z);
		break;
	}
	pNode->pChilds[0] = new BSPTreeNode;
	pNode->pChilds[0]->vMin = pNode->vMin;
	pNode->pChilds[0]->vMax = pNode->vMin + vDim;
	pNode->pChilds[1] = new BSPTreeNode;
	pNode->pChilds[1]->vMin = vMin;
	pNode->pChilds[1]->vMax = pNode->vMax;
	
	for(int i=0; i<2; i++)
	{
		GetVerticesOfAABBOfNode(pNode->pChilds[i]);
		pNode->pChilds[i]->pFaces = NULL;
		pNode->pChilds[i]->intFaceCount = nFaceCount[i];
		pNode->pChilds[i]->nType = 0;
		if(nFaceCount[i]==0) continue;
		pNode->pChilds[i]->pFaces = new FaceInfo[nFaceCount[i]];
		for(int j=0; j<nFaceCount[i]; j++)
		{
			pNode->pChilds[i]->pFaces[j].intObjectId = pFaces[i][j].intObjectId;
			pNode->pChilds[i]->pFaces[j].intFaceId = pFaces[i][j].intFaceId;
		}
	}
	delete[] pFaces[0];
	delete[] pFaces[1];
	delete[] pNode->pFaces;
	pNode->pFaces = NULL;
	CreateChildren(pNode->pChilds[0]);
	CreateChildren(pNode->pChilds[1]);
}

void CBSPTree::PartitionXY(FaceInfo* pFaces1, int* nCount1,
						   FaceInfo* pFaces2, int* nCount2,
						   FaceInfo* pFaces, int nTotalCount,float cz)
{
	BOOL bFlg[2];
	int  nCount[2];
	nCount[0] = nCount[1] = 0;
	VERTEX vxes[3];
	for(int i=0; i<nTotalCount; i++)
	{
		int objId = pFaces[i].intObjectId;
		int faceId = pFaces[i].intFaceId;
		D3DXVECTOR3 vertices[3];
		if(GetVerticesOfTri(objId,faceId,vxes)==FALSE) continue;
		bFlg[0] = bFlg[1] = FALSE;
		for(int j=0; j<3; j++)
		{
			vertices[j] = vxes[j].Point;
			float z = vertices[j].z;
			if(z < cz)	bFlg[0] = TRUE;
			else		bFlg[1] = TRUE;
		}
		if(bFlg[0])
		{
			pFaces1[nCount[0]].intFaceId = faceId;
			pFaces1[nCount[0]].intObjectId = objId;
			nCount[0]++;
		}
		if(bFlg[1])
		{
			pFaces2[nCount[1]].intFaceId = faceId;
			pFaces2[nCount[1]].intObjectId = objId;
			nCount[1]++;
		}
	}
	*nCount1 = nCount[0];
	*nCount2 = nCount[1];
}

void CBSPTree::PartitionYZ(FaceInfo* pFaces1, int* nCount1,
						   FaceInfo* pFaces2, int* nCount2,
						   FaceInfo* pFaces, int nTotalCount,float cx)
{
	BOOL bFlg[2];
	int  nCount[2];
	nCount[0] = nCount[1] = 0;
	VERTEX vxes[3];
	for(int i=0; i<nTotalCount; i++)
	{
		int objId = pFaces[i].intObjectId;
		int faceId = pFaces[i].intFaceId;
		D3DXVECTOR3 vertices[3];
		if(GetVerticesOfTri(objId,faceId,vxes)==FALSE) continue;
		bFlg[0] = bFlg[1] = FALSE;
		for(int j=0; j<3; j++)
		{
			vertices[j] = vxes[j].Point;
			float x = vertices[j].x;
			if(x < cx)	bFlg[0] = TRUE;
			else		bFlg[1] = TRUE;
		}
		if(bFlg[0])
		{
			pFaces1[nCount[0]].intFaceId = faceId;
			pFaces1[nCount[0]].intObjectId = objId;
			nCount[0]++;
		}
		if(bFlg[1])
		{
			pFaces2[nCount[1]].intFaceId = faceId;
			pFaces2[nCount[1]].intObjectId = objId;
			nCount[1]++;
		}
	}
	*nCount1 = nCount[0];
	*nCount2 = nCount[1];
}

void CBSPTree::PartitionZX(FaceInfo* pFaces1, int* nCount1,
						   FaceInfo* pFaces2, int* nCount2,
						   FaceInfo* pFaces, int nTotalCount,float cy)
{
	BOOL bFlg[2];
	int  nCount[2];
	nCount[0] = nCount[1] = 0;
	VERTEX vxes[3];
	for(int i=0; i<nTotalCount; i++)
	{
		int objId = pFaces[i].intObjectId;
		int faceId = pFaces[i].intFaceId;
		D3DXVECTOR3 vertices[3];
		if(GetVerticesOfTri(objId,faceId,vxes)==FALSE) continue;
		bFlg[0] = bFlg[1] = FALSE;
		for(int j=0; j<3; j++)
		{
			vertices[j] = vxes[j].Point;
			float y = vertices[j].y;
			if(y < cy)	bFlg[0] = TRUE;
			else		bFlg[1] = TRUE;
		}
		if(bFlg[0])
		{
			pFaces1[nCount[0]].intFaceId = faceId;
			pFaces1[nCount[0]].intObjectId = objId;
			nCount[0]++;
		}
		if(bFlg[1])
		{
			pFaces2[nCount[1]].intFaceId = faceId;
			pFaces2[nCount[1]].intObjectId = objId;
			nCount[1]++;
		}
	}
	*nCount1 = nCount[0];
	*nCount2 = nCount[1];
}

void CBSPTree::GetVerticesOfAABBOfNode(BSPTreeNode* pNode)
{
	float dx,dy,dz;
	D3DXVECTOR3 vMin, vMax;
	vMin = pNode->vMin;
	vMax = pNode->vMax;
	dx = vMax.x - vMin.x;
	dy = vMax.y - vMin.y;
	dz = vMax.z - vMin.z; 
	for(int i=0; i<4; i++)
	{
		pNode->vxes[i] = vMin;
	}
	pNode->vxes[1].x += dx;
	pNode->vxes[2].x += dx;
	pNode->vxes[2].y += dy;
	pNode->vxes[3].y += dy;
	for(int i=0; i<4; i++)
	{
		pNode->vxes[i+4] = pNode->vxes[i];
		pNode->vxes[i+4].z += dz;
	}
}

BOOL CBSPTree::CheckIntersectLeaf(BSPTreeNode* pNode, Ray* pRay)
{
	int nFaceCount = pNode->intFaceCount;
	FaceInfo* pFaces = pNode->pFaces;
	VERTEX stu_vxes[3];
	float u,v,dist,fMin = pRay->dist;
	BOOL bIntersect = FALSE;
	for(int i=0; i<nFaceCount; i++)
	{
		int objId = pFaces[i].intObjectId;
		int faceId = pFaces[i].intFaceId;
		D3DXVECTOR3 vxes[3];
		if(GetVerticesOfTri(objId,faceId,stu_vxes)==FALSE) continue;
		vxes[0] = stu_vxes[0].Point;
		vxes[1] = stu_vxes[1].Point;
		vxes[2] = stu_vxes[2].Point;
		if(D3DXIntersectTri(&vxes[0],&vxes[1],&vxes[2],&pRay->vRayPos,&pRay->vDir,&u,&v,&dist))
		{
			if(fMin > dist)
			{
				fMin = dist;
				pRay->u = u;
				pRay->v = v;
				pRay->dist = dist;
				pRay->vIntersection = vxes[0] + u*(vxes[1] - vxes[0]) + v*(vxes[2] - vxes[0]);
				pRay->nFaceIndex = faceId;
				pRay->nObjectId = objId;
				pRay->bHit = TRUE;
				bIntersect = TRUE;
				for(int k=0; k<3; k++)
				{
					pRay->txes[k].x = stu_vxes[k].u;
					pRay->txes[k].y = stu_vxes[k].v;
				}
				
				//pRay->nCount++;
			}
		}
		/*else
		{
			if(D3DXIntersectTri(&vxes[2],&vxes[1],&vxes[0],&pRay->vRayPos,&pRay->vDir,&u,&v,&dist))
			{
				if(fMin > dist)
				{
					fMin = dist;
					pRay->dist = dist;
					pRay->vIntersection = vxes[2] + u*(vxes[1] - vxes[2]) + v*(vxes[0] - vxes[2]);
					pRay->nFaceIndex = faceId;
					pRay->nObjectId = objId;
					pRay->bHit = TRUE;
					bIntersect = TRUE;
					//pRay->nCount++;
				}
			}
		}*/
	}
	return bIntersect;
}

void CBSPTree::TraverseNode(Ray* pRay)
{
	pRay->dist = FLT_MAX;
	CheckNode(m_pRoot, pRay);
}

void CBSPTree::CheckNode(BSPTreeNode* pNode, Ray* pRay)
{
	if(D3DXBoxBoundProbe(&pNode->vMin,&pNode->vMax,&pRay->vRayPos,&pRay->vDir)==FALSE) return;
	if(pNode->nType==1)
	{
		CheckIntersectLeaf(pNode,pRay);
		return;
	}
	CheckNode(pNode->pChilds[0],pRay);
	CheckNode(pNode->pChilds[1],pRay);
}

BOOL CBSPTree::GetVerticesOfTri(int objId, int face, VERTEX* vertices)
{
	VERTEX* pVertices;
	if(g_arrObjects[objId]->pVertexBuffer->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD)!=D3D_OK)
		return FALSE;
	int* pIndices;
	if(g_arrObjects[objId]->pIndexBuffer->Lock(0,0,(void**)&pIndices,D3DLOCK_DISCARD)!=D3D_OK)
		return FALSE;
	for(int i=0; i<3; i++)
	{
		vertices[i] = pVertices[pIndices[3*face+i]];
	}
	g_arrObjects[objId]->pVertexBuffer->Unlock();
	g_arrObjects[objId]->pIndexBuffer->Unlock();
	return TRUE;
}
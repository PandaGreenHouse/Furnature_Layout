#include "stdafx.h"
#include "ObjectNode.h"
#include "HitProcess.h"
#include <float.h>
#include "Model.h"
#define DELTA 80.0f

CHitProcess::CHitProcess(void)
{
	m_pPickedNode = NULL;
}

void CHitProcess::PickObject(int x, int y, CViewport3D* viewport, int viewtype, PDIRECT3DDEVICE9 pd3dDevice)
{
	m_pPickedNode = NULL;

	int nObjects = g_arrObjects.GetCount();
	ObjectNode** ppNodes = g_arrObjects.GetItem(0); 
	if(ppNodes[0]==NULL)	return;
	//viewport->MoveCamera(pd3dDevice);
	D3DXVECTOR3 vRayPos, vRayDir;
	if(viewport->GetViewMode()==PERSPECTIVE)
		GetRayDir(x,y,&vRayPos,&vRayDir,viewport,pd3dDevice);
	else
	{
		//Orthographic_MouseToWorld(&vRayPos,x,y,viewport,pd3dDevice);
		GetOrthoRay(&vRayDir,&vRayPos,x,y,viewport,pd3dDevice);
	}
	
	float fMinDist = FLT_MAX;
	int intHitted=-1;
	for(int i=0; i<nObjects; i++)
	{
		if(ppNodes[i]->pVertexBuffer==NULL) continue;
		float dist = RayHitTest(&vRayPos,&vRayDir,ppNodes[i]);
		if(dist<0.0f)
			continue;
		if(dist<fMinDist)
		{
			fMinDist = dist;
			intHitted = i;
		}
	}
	m_IntSelected = intHitted;
	ProcessHit(intHitted,viewtype);
}

void CHitProcess::GetRayDir(int ix, int iy, D3DXVECTOR3* vRayPos, D3DXVECTOR3* vRayDir, 
		CViewport3D* viewport, PDIRECT3DDEVICE9 pd3dDevice)
{
	float px = 0.0f;
	float py = 0.0f;
	RECT rect;
	viewport->GetViewRect((&rect));
	float width	 = (float) (rect.right - rect.left);
	float height = (float) (rect.bottom - rect.top);
	D3DXMATRIX proj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);
	int x,y;
	x = ix - rect.left;
	y = iy - rect.top;
	px = ((( 2.0f * x) / width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f * y) / height) + 1.0f) / proj(1, 1);
	D3DXVECTOR3 vOrigin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vDir = D3DXVECTOR3(px, py, 1.0f);

	D3DXMATRIX view;
	pd3dDevice->GetTransform(D3DTS_VIEW, &view);
	// inverse it
	D3DXMATRIX viewInverse;
	D3DXMatrixInverse(&viewInverse, 0, &view);
	D3DXVec3TransformCoord(&vOrigin,&vOrigin,&viewInverse);
	D3DXVec3TransformNormal(&vDir,&vDir,&viewInverse);
	*vRayDir = vDir;
	*vRayPos = vOrigin;
}

float CHitProcess::RayHitTest(D3DXVECTOR3* pRayPos, D3DXVECTOR3* pRayDir, ObjectNode* pNode)
{
	VERTEX* pVertices;
	if(pNode->pVertexBuffer->Lock(0,0,(void**)&pVertices, D3DLOCK_DISCARD ) != D3D_OK)
		return -1.0f;
	int* pIndices;
	if(pNode->pIndexBuffer->Lock(0,0,(void**)&pIndices, D3DLOCK_DISCARD)!=D3D_OK)
		return -1.0f;
	int nFaces = pNode->intFaceCount;
	D3DXVECTOR3 pos, dir;
	pos = *pRayPos;
	dir = *pRayDir;
	D3DXVECTOR3 v[4];
	float s,t,dist,fMin;
	fMin = FLT_MAX; //50000.0f;
	int intObjectId = -1;
	int n[3];
	bool bInterscet = false;
	for(int i=0; i<nFaces; i++)
	{
		n[0] = pIndices[3*i+0];
		n[1] = pIndices[3*i+1];
		n[2] = pIndices[3*i+2];
		v[0] = pVertices[n[0]].Point;
		v[1] = pVertices[n[1]].Point;
		v[2] = pVertices[n[2]].Point;
		if(D3DXIntersectTri(&v[0],&v[1],&v[2],pRayPos,pRayDir,&s,&t,&dist))
		{
			if(fMin>dist)
			{
				intObjectId = i;
				fMin = dist;
			}
			bInterscet = true;
		}
		if(D3DXIntersectTri(&v[2],&v[1],&v[0],pRayPos,pRayDir,&s,&t,&dist))
		{
			if(fMin>dist)
			{
				intObjectId = i;
				fMin = dist;
			}
			bInterscet = true;
		}
	}
	pNode->pVertexBuffer->Unlock();
	pNode->pIndexBuffer->Unlock();
	if(!bInterscet)
		return -1.0f;
	return fMin;
}

void CHitProcess::ProcessHit(int intHitted, int view_mode)
{
	m_pPickedNode = NULL;
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		ppNodes[i]->setAloneSelected(false);
		ppNodes[i]->bPicked = false;
	}
	if(intHitted==-1)
	{
		return;
	}
	
	if(view_mode == VIEW_TOP)
	{
		m_pPickedNode = SelecteGroupParent(intHitted);
		m_pPickedNode->setSelected(true);
		m_pPickedNode->setAloneSelected(true);
	}
	else
	{
		if(ppNodes[intHitted]->isDelete == false && ppNodes[intHitted]->isMove == false)
		{
			ppNodes[intHitted]->getParent()->setSelected(true);
			ppNodes[intHitted]->getParent()->bPicked = true;
		}
		else
		{
			ppNodes[intHitted]->setAloneSelected(true);
			ppNodes[intHitted]->bPicked = true;
		}
	}
}

void CHitProcess::Perspective_MouseToWorld(D3DXVECTOR3* pWorldPos, int ix, int iy, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	float px = 0.0f;
	float py = 0.0f;
	RECT rect;
	viewport->GetViewRect((&rect));
	float width	 = (float) (rect.right - rect.left);
	float height = (float) (rect.bottom - rect.top);
	D3DXMATRIX proj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);
	float x,y;
	x = (float) (ix - rect.left);
	y = (float) (iy - rect.top);
	px = ((( 2.0f * x) / width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f * y) / height) + 1.0f) / proj(1, 1);
	D3DXVECTOR3 vOrigin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vDir = D3DXVECTOR3(px, py, 1.0f);

	D3DXMATRIX view;
	pd3dDevice->GetTransform(D3DTS_VIEW, &view);
	// inverse it
	D3DXMATRIX viewInverse;
	D3DXMatrixInverse(&viewInverse, 0, &view);
	D3DXVec3TransformCoord(&vOrigin,&vOrigin,&viewInverse);
	D3DXVec3TransformNormal(&vDir,&vDir,&viewInverse);
	*pWorldPos = vOrigin + vDir;
}

void CHitProcess::Orthographic_MouseToWorld(D3DXVECTOR3* pWorldPos, int ix, int iy, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	float px = 0.0f;
	float py = 0.0f;
	RECT rect;
	viewport->MoveCamera(pd3dDevice);
	viewport->GetViewRect((&rect));
	float width	 = (float) (rect.right - rect.left);
	float height = (float) (rect.bottom - rect.top);
	D3DXMATRIX proj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &proj);
	float x,y;
	x = (float) (ix - rect.left);
	y = (float) (iy - rect.top);
	px = ((( 2.0f * x) / width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f * y) / height) + 1.0f) / proj(1, 1);
	D3DXVECTOR3 vPos = D3DXVECTOR3(px, py, 1.0f);

	D3DXMATRIX view;
	pd3dDevice->GetTransform(D3DTS_VIEW, &view);
	// inverse it
	D3DXMATRIX viewInverse;
	D3DXMatrixInverse(&viewInverse, 0, &view);
	D3DXVec3TransformCoord(&vPos,&vPos,&viewInverse);
	*pWorldPos = vPos;
}

void CHitProcess::GetOrthoRay(D3DXVECTOR3* pRayDir, D3DXVECTOR3*pRayPos, int ix, int iy, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	D3DXVECTOR3 vPos, vDir;
	Orthographic_MouseToWorld(&vPos, ix, iy, viewport, pd3dDevice);
	viewport->GetCameraDir(&vDir);
	*pRayDir = vDir;
	*pRayPos = vPos;
}
/*
translate the selected objects and the origin of the picked object is transformed 
*/
void CHitProcess::MoveObject(POINT* prevPt, POINT* curPt, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	viewport->MoveCamera(pd3dDevice);
	D3DXVECTOR3 vPos[2];
	Orthographic_MouseToWorld(&vPos[0], prevPt->x, prevPt->y, viewport, pd3dDevice);
	Orthographic_MouseToWorld(&vPos[1], curPt->x, curPt->y, viewport, pd3dDevice);
	
	D3DXVECTOR3 vTranslation = vPos[1] - vPos[0];
	D3DXMATRIX  matTranslation;
	D3DXMatrixTranslation(&matTranslation,vTranslation.x,vTranslation.y,vTranslation.z);
	int nObjects = g_arrObjects.GetCount();
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	D3DXVECTOR4 vec4;
	for(int i=0; i<nObjects; i++)
	{
		//if(!ppNodes[i]->pVertexBuffer) continue;
		if(!ppNodes[i]->isSelected()) continue;
		ppNodes[i]->vOrg.x += vTranslation.x; 
		ppNodes[i]->vOrg.y += vTranslation.y; 
		ppNodes[i]->vOrg.z += vTranslation.z;

		TranformObject(ppNodes[i], &matTranslation);
	}
}

void CHitProcess::RotateObject(float fRadian)
{
	if(m_pPickedNode==NULL)	return;
	D3DXMATRIX rotMat;
	//D3DXMatrixRotationZ(&rotMat, D3DX_PI/2);
	D3DXVECTOR4 vec4;
	D3DXVECTOR3 vRotCenter;
	D3DXQUATERNION QRotation;
	QRotation.x = 0.0f;
	QRotation.y = 0.0f;
	QRotation.z = sin(fRadian/2);
	QRotation.w = cos(fRadian/2);
	vRotCenter.x = m_pPickedNode->x;/*set the origin of the object to the rotation center*/
	vRotCenter.y = m_pPickedNode->y;
	vRotCenter.z = m_pPickedNode->z;
	D3DXMatrixTransformation(
		&rotMat,
		NULL,
		NULL,
		NULL,
		&vRotCenter,
		&QRotation,
		NULL
		);
	D3DXMATRIX rotMatZ;
	D3DXMatrixRotationZ(&rotMatZ, fRadian);
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		//if(!ppNodes[i]->pVertexBuffer) continue;
		if(ppNodes[i]->isSelected())
		{
			ppNodes[i]->za += fRadian; 
			TranformObject(ppNodes[i],&rotMat);
			RotateNormals(ppNodes[i],&rotMatZ);
		}
	}

}

void CHitProcess::TranformObject(ObjectNode* pNode, D3DXMATRIX* mat)
{
	VERTEX* pVertices;
	D3DXVECTOR4 vec4;
	D3DXVECTOR3 vOrigin = D3DXVECTOR3(pNode->x,pNode->y,pNode->z);
	D3DXVec3Transform(&vec4,&vOrigin,mat);
	pNode->x = vec4.x;
	pNode->y = vec4.y;
	pNode->z = vec4.z;
	for(int i=0; i<6; i++)
	{
		for(int j=0; j<4; j++)
		{
			D3DXVec3Transform(&vec4,&pNode->vBounds[i][j],mat);
			pNode->vBounds[i][j].x = vec4.x;
			pNode->vBounds[i][j].y = vec4.y;
			pNode->vBounds[i][j].z = vec4.z;
		}
	}
	LineVertex* pLineVertices;
	if(pNode->pLineVertexBuffer==NULL) return;
	if(pNode->pLineVertexBuffer->Lock(0,0,(void**)&pLineVertices,D3DLOCK_DISCARD)!=D3D_OK) return;
	for(int i=0; i<pNode->intLineVtCount; i++)
	{
		D3DXVec3Transform(&vec4,&pLineVertices[i].vertex,mat);
		pLineVertices[i].vertex.x = vec4.x;
		pLineVertices[i].vertex.y = vec4.y;
		pLineVertices[i].vertex.z = vec4.z;
	}
	pNode->pLineVertexBuffer->Unlock();
	if(pNode->pVertexBuffer==NULL) return;
	if(pNode->pVertexBuffer->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD)!=D3D_OK) return;
	for(int j=0; j<pNode->intVtCount; j++)
	{
		D3DXVec3Transform(&vec4,&pVertices[j].Point,mat);
		pVertices[j].Point.x = vec4.x;
		pVertices[j].Point.y = vec4.y;
		pVertices[j].Point.z = vec4.z;
	}
	pNode->pVertexBuffer->Unlock();
}

float CHitProcess::GetMinDistance(D3DXVECTOR2* pDist, ObjectNode* pNode1, ObjectNode* pNode2)
{
	float fMin = FLT_MAX, length;
	LPVERTEX pVertices1, pVertices2;
	D3DXVECTOR2 vert2D[2];
	D3DXVECTOR2 vDist;
	int intVtCount1, intVtCount2;
	intVtCount1 = pNode1->intVtCount;
	intVtCount2 = pNode2->intVtCount;
	if(pNode1->pVertexBuffer==NULL || pNode2->pVertexBuffer == NULL) return FLT_MAX;
	if(pNode1->pVertexBuffer->Lock(0,0,(void**)&pVertices1,D3DLOCK_DISCARD)!=D3D_OK) return FLT_MAX;
	if(pNode2->pVertexBuffer->Lock(0,0,(void**)&pVertices2,D3DLOCK_DISCARD)!=D3D_OK) return FLT_MAX;
	for(int i=0; i<intVtCount2; i++)
	{
		vert2D[1].x = pVertices2[i].Point.x;
		vert2D[1].y = pVertices2[i].Point.y;
		for(int j=0; j<intVtCount1; j++)
		{
			vert2D[0].x = pVertices1[j].Point.x;
			vert2D[0].y = pVertices1[j].Point.y;
			vDist = vert2D[1] - vert2D[0];
			length = D3DXVec2Length(&vDist);
			if(length < fMin)
			{
				fMin = length;
				*pDist = vDist;
			}
		}
	}
	pNode1->pVertexBuffer->Unlock();
	pNode2->pVertexBuffer->Unlock();
	return fMin;
}

void CHitProcess::WeldingObjects()
{
	if(m_pPickedNode==NULL)	return;
	ProcessOfOverlap();
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	int nObjects = g_arrObjects.GetCount();
	D3DXVECTOR2 vDist, vTranslate2D;
	float fMin = FLT_MAX, length;
	for(int i=0; i<nObjects; i++)
	{
		if(!ppNodes[i]->isSelected()) continue;
		for(int j=0; j<nObjects; j++)
		{
			if(ppNodes[j]->isSelected()) continue;
			length = GetMinDistance(&vDist,ppNodes[i],ppNodes[j]);
			if(length < fMin)
			{
				fMin = length;
				vTranslate2D = vDist;
			}
		}
	}
	if(fMin < DELTA)
	{
		D3DXMATRIX mat;  
		D3DXMatrixTranslation(&mat,vTranslate2D.x,vTranslate2D.y,0.0f);
		for(int i=0; i<nObjects; i++)
		{
			if(ppNodes[i]->isSelected())
			{
				TranformObject(ppNodes[i],&mat);
			}
		}
	}
	/*ObjectNode* pSibling = m_pPickedNode->getParent()->getChild();
	while(pSibling)
	{
		if(pSibling==m_pPickedNode)	
		{
			pSibling = pSibling->getSibling();
			continue;
		}
		length = GetMinDistance(&vDist,m_pPickedNode,pSibling);
		pSibling = pSibling->getSibling();
		if(length<fMin)
		{
			fMin = length;
			vTranslate2D = vDist;
		}
	}
	if(fMin < DELTA)
	{
		D3DXMATRIX mat;  
		D3DXMatrixTranslation(&mat,vTranslate2D.x,vTranslate2D.y,0.0f);
		TranformObject(m_pPickedNode,&mat);
	}*/
}

ObjectNode* CHitProcess::SelecteGroupParent(int intHitted)
{
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	ObjectNode* pNode = ppNodes[intHitted];
	ObjectNode* pParent;
	while(pNode!=ppNodes[0])
	{
		pParent = pNode;
		pNode = pNode->getParent();
	}
	pParent->bPicked = true;
	return pParent;
}
/*
node1:targetNode
node2:sourceNode
*/
bool CHitProcess::IsOverLapped(ObjectNode* node0, ObjectNode* node1, D3DXVECTOR2* vNearPos)
{
	RECT rect[2];
	float minX[2], maxX[2], minY[2], maxY[2];
	for(int i=0; i<2; i++)
	{
		minX[i] = minY[i] = FLT_MAX;
		maxX[i] = maxY[i] = FLT_MIN;
	}
	/*get boundingboxes*/
	for(int i=0; i<4; i++)
	{
		if(node0->vBounds[4][i].x < minX[0])
		{
			minX[0] = node0->vBounds[4][i].x;
		}
		if(node0->vBounds[4][i].x > maxX[0])
		{
			maxX[0] = node0->vBounds[4][i].x;
		}
		if(node0->vBounds[4][i].y < minY[0])
		{
			minY[0] = node0->vBounds[4][i].y;
		}
		if(node0->vBounds[4][i].y > maxY[0])
		{
			maxY[0] = node0->vBounds[4][i].y;
		}

		if(node1->vBounds[4][i].x < minX[1])
		{
			minX[1] = node1->vBounds[4][i].x;
		}
		if(node1->vBounds[4][i].x > maxX[1])
		{
			maxX[1] = node1->vBounds[4][i].x;
		}
		if(node1->vBounds[4][i].y < minY[1])
		{
			minY[1] = node1->vBounds[4][i].y;
		}
		if(node1->vBounds[4][i].y > maxY[1])
		{
			maxY[1] = node1->vBounds[4][i].y;
		}
	}
	rect[0].left = (int)minX[0];
	rect[0].right = (int)maxX[0];
	rect[0].top = (int)minY[0];
	rect[0].bottom = (int)maxY[0];
	rect[1].left = (int)minX[1];
	rect[1].right = (int)maxX[1];
	rect[1].top = (int)minY[1];
	rect[1].bottom = (int)maxY[1];
	POINT pts[2][5];
	for(int i=0; i<2; i++)
	{
		pts[i][0].x = rect[i].left;
		pts[i][0].y = rect[i].top;
		pts[i][1].x = rect[i].left;
		pts[i][1].y = rect[i].bottom;
		pts[i][2].x = rect[i].right;
		pts[i][2].y = rect[i].bottom;
		pts[i][3].x = rect[i].right;
		pts[i][3].y = rect[i].top;
		pts[i][4].x = (rect[i].left+rect[i].right)/2;
		pts[i][4].y = (rect[i].top + rect[i].bottom)/2;
	}
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<5; j++)
		{
			if(PtInRect(&rect[i],pts[1-i][j]))
			{
				*vNearPos = GetNearCenterPoint(&rect[0], &rect[1]);
				return true;
			}
		}
	}
	return false;
}

D3DXVECTOR2 CHitProcess::GetNearCenterPoint(RECT* pTargetRect, RECT* pSrcRect)
{
	D3DXVECTOR2 targetCenter,targetPoints[4];
	targetCenter.x = (float)(pTargetRect->right + pTargetRect->left)/2;
	targetCenter.y = (float)(pTargetRect->bottom + pTargetRect->top)/2;
	for(int i=0; i<4; i++)
	{
		targetPoints[i] = targetCenter;
	}
	int width = (pTargetRect->right - pTargetRect->left)/2 + (pSrcRect->right - pSrcRect->left)/2;
	int height = (pTargetRect->bottom - pTargetRect->top)/2 + (pSrcRect->bottom - pSrcRect->top)/2;
	targetPoints[0].x -= (float)width; 
	targetPoints[1].x += (float)width; 
	targetPoints[2].y -= (float)height;
	targetPoints[3].y += (float)height;
	D3DXVECTOR2 srcCenter, vec2;
	srcCenter.x = (float)(pSrcRect->left + pSrcRect->right)/2;
	srcCenter.y = (float)(pSrcRect->top + pSrcRect->bottom)/2;
	float length, fMin = FLT_MAX;
	int nSelected = 0;
	for(int i=0; i<4; i++)
	{
		vec2 = targetPoints[i] - srcCenter;
		length = D3DXVec2Length(&vec2);
		if(length < fMin)
		{
			fMin = length;
			nSelected = i;
		}
	}
	return targetPoints[nSelected];
}

void CHitProcess::ProcessOfOverlap()
{
	if(m_pPickedNode==NULL) return;
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	ObjectNode* pSibling = ppNodes[0]->getChild();
	D3DXVECTOR3 vTargetPos;
	D3DXVECTOR3 vCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<4; j++)
		{
			vCenter.x += m_pPickedNode->vBounds[i][j].x;
			vCenter.y += m_pPickedNode->vBounds[i][j].y;
			vCenter.z += m_pPickedNode->vBounds[i][j].z;
		}
	}
	vCenter.x /= 8;
	vCenter.y /= 8;
	vCenter.z /= 8;
	while(pSibling)
	{
		if(pSibling!=m_pPickedNode)
		{
			//if(IsOverLapped(pSibling,m_pPickedNode,&vTargetPos))
			if(DetectingInterference(m_pPickedNode,pSibling,&vTargetPos))
			{
				D3DXVECTOR3 vTranslate = vTargetPos - vCenter;
				D3DXMATRIX mat;  
				D3DXMatrixTranslation(&mat,vTranslate.x,vTranslate.y,vTranslate.z);
				TransformAllSelected(&mat);
				//TranformObject(m_pPickedNode,&mat);
				return;
			}
		}
		pSibling = pSibling->getSibling();
	}
}

void CHitProcess::LocatePickedObject()
{
	if(m_pPickedNode==NULL) return;
	ProcessOfOverlap();	
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	ObjectNode* pSibling = ppNodes[0]->getChild();
	float length, fMin = FLT_MAX;
	D3DXVECTOR3 vert3D[2][8];
	int k=0;
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<4; j++)
		{
			vert3D[0][k] = m_pPickedNode->vBounds[i][j];
			k++;
		}
	}

	D3DXVECTOR3 vTranslate;
	while(pSibling)
	{
		if(pSibling!=m_pPickedNode)
		{
			for(int k=0; k<8; k++)
			{
				for(int i=0; i<2; i++)
				{
					for(int j=0; j<4; j++)
					{
						D3DXVECTOR3 vec3 = pSibling->vBounds[i][j] - vert3D[0][k];
						length = D3DXVec3Length(&vec3);
						if(length < fMin)
						{
							fMin = length;
							vTranslate = vec3;
						}
					}
				}
			}
		}
		pSibling = pSibling->getSibling();
	}
	if(fMin < DELTA)
	{
		D3DXMATRIX mat;
		D3DXMatrixTranslation(&mat,vTranslate.x,vTranslate.y,vTranslate.z);
		TransformAllSelected(&mat);
	}
}

void CHitProcess::TransformAllSelected(D3DXMATRIX* mat)
{
	int nObjects = g_arrObjects.GetCount() ;
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	for(int i=0; i<nObjects; i++)
	{
		if(!ppNodes[i]->isSelected()) continue;
		TranformObject(ppNodes[i], mat);
	}	
}

bool CHitProcess::DetectingInterference(ObjectNode* node1, ObjectNode* node2, D3DXVECTOR3* vPos)
{
	D3DXVECTOR3 vNormals[6], vDir, vCenter, vec[2], vert3D[9];
	int k=0;
	/*get the center and bounding box vertices of node1*/
	/*vCenter.x = node1->x + node1->w/2;
	vCenter.y = node1->y + node1->d/2;
	vCenter.z = node1->z + node1->h/2;*/
	vCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<4; j++)
		{
			vert3D[k] = node1->vBounds[i][j];
			vCenter += node1->vBounds[i][j];
			k++;
		}
	}
	vert3D[8] = vCenter/8;
	/*Check if the bounding box's vertices of the node1 exist in the bounding box of the node2*/
	vCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	int count = 0;
	bool bInterference = false;
	for(int i=0; i<9; i++)
	{
		count = 0;
		for(int j=0; j<6; j++)
		{
			vCenter = (node2->vBounds[j][0] + node2->vBounds[j][1] + node2->vBounds[j][2] + node2->vBounds[j][3])/4;
			vec[0] = node2->vBounds[j][1] - node2->vBounds[j][0];
			vec[1] = node2->vBounds[j][2] - node2->vBounds[j][0];
			D3DXVec3Cross(&vNormals[j],&vec[0],&vec[1]);
			D3DXVec3Normalize(&vNormals[j],&vNormals[j]);
			vDir = vCenter - vert3D[i];
			float dot = D3DXVec3Dot(&vDir,&vNormals[j]);
			if(dot < 0.0f) break;
			count++;
		}
		if(count==6)
		{
			bInterference = true;
			break;
		}
	}
	if(bInterference==false) return false; 
	/*calculate the near points around the center */
	D3DXVECTOR3 vertices[6];
	/*vCenter.x = node2->x + node2->w/2;
	vCenter.y = node2->y + node2->d/2;
	vCenter.z = node2->z + node2->h/2;*/
	vCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<4; j++)
		{
			vCenter += node2->vBounds[i][j];
		}
	}
	vCenter /= 8;
	for(int i=0; i<6; i++)
	{
		vertices[i] = vCenter;
	}
	vertices[0].y -= node2->d/2 + node1->d/2;
	vertices[1].y += node2->d/2 + node1->d/2;
	vertices[2].x -= node2->w/2 + node1->w/2;
	vertices[3].x += node2->w/2 + node1->w/2;
	vertices[4].z += node2->h/2 + node1->h/2;
	vertices[5].z -= node2->h/2 + node1->h/2;
	/*get the nearest point to the center of the node1*/
	float fMin = FLT_MAX, length;
	for(int i=0; i<6; i++)
	{
		D3DXVECTOR3 vec = vert3D[8] - vertices[i];
		length = D3DXVec3Length(&vec);
		if(length < fMin)
		{
			fMin = length;
			*vPos = vertices[i];
		}
	}
	return true;
}

void CHitProcess::RotateNormals(ObjectNode* pNode, D3DXMATRIX* rotMat)
{
	if(pNode->pVertexBuffer==NULL) return;
	VERTEX* pVertices = NULL;
	D3DXVECTOR4 vec4; 
	if(pNode->pVertexBuffer->Lock(0,0,(void**)&pVertices,D3DLOCK_DISCARD)!=D3D_OK) return;
	for(int j=0; j<pNode->intVtCount; j++)
	{
		D3DXVec3Transform(&vec4,&pVertices[j].Normal,rotMat);
		pVertices[j].Normal.x = vec4.x;
		pVertices[j].Normal.y = vec4.y;
		pVertices[j].Normal.z = vec4.z;
	}
	pNode->pVertexBuffer->Unlock();	
}

void CHitProcess::ScatteringObjects(float s)
{
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	int nObjects = g_arrObjects.GetCount();
	D3DXVECTOR3 vTranslate;
	D3DXMATRIX matTranslate;
	ppNodes[0]->fDecomp += s;
	if(ppNodes[0]->fDecomp < 1.0f)
	{
		ppNodes[0]->fDecomp = 1.0f;
		return;
	}
	for(int i=1; i<nObjects; i++)
	{
		vTranslate.x = ppNodes[i]->cx*s;
		vTranslate.y = ppNodes[i]->cy*s;
		vTranslate.z = ppNodes[i]->cz*s;
		D3DXMatrixTranslation(&matTranslate,vTranslate.x,vTranslate.y,vTranslate.z);
		TranformObject(ppNodes[i],&matTranslate);
	}
}
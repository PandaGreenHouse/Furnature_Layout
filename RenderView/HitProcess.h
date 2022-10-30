#pragma once
#include "Viewport3D.h"
#include "ObjectNode.h"

class CHitProcess
{
public:
	CHitProcess(void);
	void PickObject(int x, int y, CViewport3D* viewport, int viewtype, PDIRECT3DDEVICE9 pd3dDevice);
	void GetRayDir(int x, int y, D3DXVECTOR3* vRayPos, D3DXVECTOR3* vRayDir, 
		CViewport3D* viewport, PDIRECT3DDEVICE9 pd3dDevice);
	void Orthographic_MouseToWorld(D3DXVECTOR3* pWorldPos, int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void MoveObject(POINT* prevPt, POINT* curPt, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void GetOrthoRay(D3DXVECTOR3* pRayDir, D3DXVECTOR3*pRayPos, int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void RotateObject(float fRadian);
	void WeldingObjects();
	void LocatePickedObject();
	void ScatteringObjects(float s);
protected:
	float RayHitTest(D3DXVECTOR3* pRayPos, D3DXVECTOR3* pRayDir, ObjectNode* pNode);
	void ProcessHit(int intHitted, int view_mode);
	void Perspective_MouseToWorld(D3DXVECTOR3* pWorldPos, int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void TranformObject(ObjectNode* pNode, D3DXMATRIX* mat);
	ObjectNode* SelecteGroupParent(int intHitted);
	float GetMinDistance(D3DXVECTOR2* vDist, ObjectNode* pNode1, ObjectNode* pNode2);
	bool IsOverLapped(ObjectNode* node1, ObjectNode* node2, D3DXVECTOR2* vec2);
	D3DXVECTOR2 GetNearCenterPoint(RECT* pRect1, RECT* pRect2);
	void ProcessOfOverlap();
	void TransformAllSelected(D3DXMATRIX* mat);
	bool DetectingInterference(ObjectNode* node1, ObjectNode* node2, D3DXVECTOR3* vPos);
	void RotateNormals(ObjectNode* node, D3DXMATRIX* rotMat);
	//Attributes
private:
	ObjectNode* m_pPickedNode;
	int m_IntSelected;
	int m_IntPicked;
};


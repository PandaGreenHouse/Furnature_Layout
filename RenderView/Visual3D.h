#pragma once
#include "ObjectNode.h"
class CVisual3D
{
public:
	CVisual3D(void);
	void CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice);
	void Render(LPDIRECT3DDEVICE9 pd3dDevice, int mode);
	void Release();
private:
	void FullRender(LPDIRECT3DDEVICE9 pd3dDevice);
	void WireframeRender(LPDIRECT3DDEVICE9 pd3dDevice);
	void ShapeLineRender(LPDIRECT3DDEVICE9 pd3dDevice);
	void ToScreenCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint);
	void GetMatrixToProjectCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX* pMat);
	void RenderShape(LPDIRECT3DDEVICE9 pd3dDevice);
	void RenderPickedPlanks(LPDIRECT3DDEVICE9 pd3dDevice);
	void DrawObject3DLine(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, DWORD color );
	void DrawObject2DLine(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, DWORD color);
	//attributes
private:
	LPD3DXLINE	m_pLine;
	D3DXMATRIX  m_MatWorld;
	D3DXVECTOR3 m_vScalingCenter;
	D3DXVECTOR3 m_vScaling;
};

extern CVisual3D gClsVisual;

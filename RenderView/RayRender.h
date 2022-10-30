#pragma once
#include "BSPTree.h"
class CRayRender
{
public:
	CRayRender(void);
	~CRayRender(void);
//Attributes
	CBSPTree	m_ClsBSPTree;
	LPD3DXCOLOR m_pColors;
	int			m_nShadeType;	
//Implementation
	BOOL LockBuffersOfObject(VERTEX** pVertices, int** pIndices, int objId);
	void UnlockBuffersOfObject(int objId);
	void SetPointLight();
	void SetShadeType(int nType);
	void CreateTree();
	void RT_RenderScene(HDC hDC, int Nx, int Ny, BOOL bRender);
	void Smoothing(D3DXCOLOR* pColors, int width, int height);
	void DrawPixels(HDC hDC, LPD3DXCOLOR pColors, int width, int height);
	BOOL GetNearestHittedPoint(Ray* ray);
	BOOL GetReflectedRay(Ray* pReflectedRay, Ray* pRay);
	void GetRayHitData(D3DXVECTOR3* vertices, D3DXVECTOR3* normals, D3DXVECTOR3* hittedPoint, Ray* pRay);
	float PhongShading(Ray* pRay);
	BOOL CheckRayIntersection(Ray* ray);
	BOOL CheckShadow(D3DXVECTOR3* vPos);
	int GetIntersectionData(D3DXVECTOR3* vPos, D3DXVECTOR3* vNormal, int* faceId, Ray* ray);
	D3DXCOLOR RayTracing(Ray* ray);
	float LambertIntensity(Ray* ray);
	float GouraudIntensity(Ray* ray);
	float PhongIntensity(Ray* ray);
	D3DXCOLOR GetTextureColor(Ray* ray);
};


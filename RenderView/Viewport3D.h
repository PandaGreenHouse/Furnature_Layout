#pragma once
#include "Visual3D.h"

class CViewport3D
{
public:
	CViewport3D(void);
	void Release();
	void AddVisual3D(CVisual3D* visual3D);
	void Delete(int element);
	void Render(LPDIRECT3DDEVICE9 pd3dDevice, int intRendermode, bool bSeleted, RECT* pRect);
	void InitCamera(Camera* pCamera);
	void CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice);
	void GetViewRect(RECT* pRect);
	void SetViewport(LPDIRECT3DDEVICE9 pd3dDevice, RECT* pRect);
	void RotateCamera(float rotX, float rotY);
	int GetViewMode() { return m_StrCamera.mode;};
	void ZoomIn(D3DXVECTOR3* v);
	void ZoomOut(D3DXVECTOR3* v);
	void MoveCamera(LPDIRECT3DDEVICE9 pd3dDevice);
	void GetCameraDir(D3DXVECTOR3* vDir);
	void SetDrawStrokes(bool bflg) {m_bStroke = bflg;};
	void SetDrawText(bool bflg) {m_bDrawText = bflg;};
	void SetTitle(TCHAR* pStr){_tcscpy(m_szTitle, pStr);}
	void RotateObject(float fSpinX, float fSpinY, float fSpinZ);
	void MoveObject(D3DXVECTOR3* vPos);
	void ScalingObject(D3DXVECTOR3* vScaleCenter, float fScaleX, float fScaleY, float fScaleZ);
	void SetProjectMode(int mode) {m_IntProjectMode = mode;};
	int  GetProjectMode() {return m_IntProjectMode;};
	void InitializeViewport(RECT* pRect);
private:
	void DrawViewportLines(bool bSelected, RECT* pRect);
	void PerspectiveZoomIn(D3DXVECTOR3* vDir);
	void PerspectiveZoomOut(D3DXVECTOR3* VdIR);
	void OrthographyZoomIn(D3DXVECTOR3* pTargetPos);
	void OrthographyZoomOut(D3DXVECTOR3* pTargetPos);
	void ToScreenCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint);
	void DrawStrokes(LPDIRECT3DDEVICE9 pd3dDevice);
	void DrawText(LPDIRECT3DDEVICE9 pd3dDevice);
	void DrawTitle();
	void AdjustCamera();
	//Attribute
private:
	TCHAR			  m_szTitle[32];
	ID3DXFont*		  m_pFont;
	ID3DXFont*		  m_pTitleFont;
	LPD3DXLINE		  m_pLine;
	Camera			  m_StrCamera;	
	int				  m_intCount;
	RECT			  m_ViewRect;
	RECT			  m_PrevRect;	
	bool			  m_bStroke;	
	bool			  m_bDrawText;
	Transformation    m_StrRtTranform;
	int				  m_IntProjectMode;	
	POINT			  m_Center2D;
	bool			  m_bTotal;	
};


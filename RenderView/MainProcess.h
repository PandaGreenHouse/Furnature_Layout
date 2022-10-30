#pragma once

#include "Viewport3D.h"
#include "HitProcess.h"
#include "LineProcess.h"

class CMainProcess
{
public:
	bool		m_bModel;
	CMainProcess(void);
	~CMainProcess();
	void SetHWnd(HWND hWnd){m_hWnd = hWnd;};
	HWND GetHWnd(){return m_hWnd;};

	void LoadXmlModel();
	void RenderScene();
	void SetViewMode(int mode);
	void SetRenderMode(int mode);
	void OnMouseMove(POINT* pt);
	void OnLButtonDown(POINT* pt);
	void OnLButtonUp(POINT* pt);
	void OnRButtonDown(POINT* pt);
	void ScrollDown(POINT* pt);
	void ScrollUp(POINT* pt);
	bool SetTexture(int matId);
	void SetLineDrawing(BOOL bFlag);
	void RotatePickedObject();
	void ReSize();
	void ScatterObjects(int nCount);
private:
	BOOL Initd3dDevice();
	bool Release();
	void SetViewport();
	void InitCamera();
	void PickObject(int x, int y, CViewport3D* viewport);
	void GetRayDir(int x, int y, D3DXVECTOR3* vRayPos, D3DXVECTOR3* vRayDir, CViewport3D* viewport);
	void AddPoint(int x, int y, CViewport3D* viewport);
	void AddLine();
	VOID SetPointLight(LPDIRECT3DDEVICE9 d3dDevice);	
	//Attributes
private:
	HWND				m_hWnd;

	LPDIRECT3D9         m_pD3D;				// Used to create the D3DDevice
	LPDIRECT3DDEVICE9   m_pd3dDevice;

	CViewport3D		m_ClsViewport[4];	//0:top, 1:front, 2:left, 3:perspective 
	CHitProcess		m_ClsHitProc;
	CLineProcess	m_ClsLineProc;
	Camera			m_StrCameras[4];

	RECT		m_ClientRect;
	int			m_intViewMode;
	int			m_intRenderMode;
	int			m_intSelected;
	bool		m_bselected[4];
	bool		m_bMousing;
	POINT		m_LastPt;
	
	BOOL		m_bLineDrawing;
};


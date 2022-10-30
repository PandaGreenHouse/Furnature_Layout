#pragma once
#include "MyArray.h"
#include "Viewport3D.h"
#include "HitProcess.h"
class CLineProcess
{
public:
	CLineProcess(void);
	~CLineProcess();
	void CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice);
	void AddPoint(int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void SetPoint(int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice);
	void AddLine();
private:
	void ToScreenCoord(CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint);
	void MouseToProject(CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice, POINT* pt, D3DXVECTOR2* vert2D);
protected:
	CHitProcess			m_ClsHitProc;
	int					m_intLineCount;
	int					m_intPtCount;
	LPD3DXLINE			m_pLine;
	POINT				m_CurPt;
	bool				m_bAdded;
};


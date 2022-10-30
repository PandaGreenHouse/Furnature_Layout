#include "stdafx.h"
#include "MainProcess.h"
#include "ModelObject.h"
#include "RenderView.h"
#include "Model.h"

CvvImage m_cvvImage;
CVisual3D		gClsVisual;
CModelObject	g_ClsModelObject;
D3DXVECTOR3		_vUp[4] = {D3DXVECTOR3(0.0f,-1.0f,0.0f),D3DXVECTOR3(0.0f,0.0f,1.0f),D3DXVECTOR3(0.0f,0.0f,1.0f),D3DXVECTOR3(0.0f,0.0f,1.0f)};
/*
在(4000,4000,4000)(-4000,4000,4000)(4000,-4000,4000)(-4000,-4000,4000)放4个。
在(4000,4000,-4000)(-4000,4000,-4000)(4000,-4000,-4000)(-4000,-4000,-4000)放4个，比之前4个稍微暗一些。

*/
D3DXVECTOR3 _vLightPos[8] = {D3DXVECTOR3(4000,4000,4000),D3DXVECTOR3(-4000,4000,4000),D3DXVECTOR3(4000,4000,-4000),D3DXVECTOR3(-4000,4000,-4000),
							D3DXVECTOR3(4000,-4000,-4000),D3DXVECTOR3(-4000,-4000,-4000),D3DXVECTOR3(4000,-4000,4000),D3DXVECTOR3(-4000,-4000,4000)};

void GetScreenResolution(LONG* pnWidth, LONG* pnHeight);
extern TCHAR* _copy_char2tchar(TCHAR* dst, char* src)
{
	if( src == NULL || dst == NULL)
		return NULL;
	if( src[0] == 0)
	{	dst[0] = 0;  return dst;  }

#ifdef _UNICODE
	MultiByteToWideChar( CP_THREAD_ACP, MB_COMPOSITE, src, -1, dst, (int)strlen(src)+1);
#else
	strcpy( dst, src);
#endif
	return dst;
}

extern char* _copy_tchar2char(char* dst, TCHAR* src)
{
	if( src == NULL || dst == NULL)
		return NULL;
	if( src[0] == 0)
	{	dst[0] = 0;  return dst;  }

#ifdef _UNICODE
	WideCharToMultiByte( CP_ACP, 0, src, -1, dst, (int)_tcslen(src)*sizeof(TCHAR)+1, NULL, NULL);
#else
	strcpy( dst, src);
#endif
	return dst;
}

CMainProcess::CMainProcess(void)
{
	m_hWnd = NULL;
	m_pD3D = NULL;
	m_pd3dDevice = NULL;

	m_bModel = false;

	m_bLineDrawing = FALSE;
}

CMainProcess::~CMainProcess()
{
	Release();
	m_bModel = false;
}

//Initialize D3D Device
//Create Device when only new window size is larger than m_DevRenderSize
//If Create Device Calls, Returns TRUE
BOOL CMainProcess::Initd3dDevice()
{
	BOOL	bResult = FALSE;
	SIZE	csNew;
	DWORD	dwTotal = 0;
	HRESULT	hr;
	D3DDISPLAYMODE	d3ddm;
	D3DPRESENT_PARAMETERS	d3dpp;

	//Create IDirect3D Object
	if (m_pD3D == NULL)
		m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == NULL)
		return E_FAIL;

	//Get Display Mode Info
	m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

	//Initialize D3D Presentation Parameters
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	GetScreenResolution(&(csNew.cx), &(csNew.cy));
	d3dpp.Windowed			= TRUE;
	d3dpp.SwapEffect		= D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat	= d3ddm.Format;
	d3dpp.BackBufferWidth	= csNew.cx;
	d3dpp.BackBufferHeight	= csNew.cy;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
	//d3dpp.MultiSampleType = D3DMULTISAMPLE_6_SAMPLES;//added for antialiasing 2014.11.21
	if (SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
							d3ddm.Format, TRUE, D3DMULTISAMPLE_NONMASKABLE,&dwTotal)))
	{    
		d3dpp.MultiSampleType		= D3DMULTISAMPLE_NONMASKABLE;
		d3dpp.MultiSampleQuality	= dwTotal - 1;
	}
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	//Create D3D Device
	if (m_pd3dDevice == NULL)
	{
		hr = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice);
		if (SUCCEEDED(hr))
			bResult = TRUE;
	}
	if (m_pd3dDevice == NULL)
		return FALSE;

	//Initialize D3D Device Environments
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	m_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE );//added for antialiasing 2014.11.21
	SetPointLight(m_pd3dDevice);

	InitCamera();
	SetViewport();

	return bResult;
}

bool CMainProcess::Release()
{
	if (m_pD3D) 
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	if (m_pd3dDevice)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	for (int i=0; i<4; i++)
	{
		m_ClsViewport[i].Release();
	}

	return true;
}

void CMainProcess::LoadXmlModel()
{
	m_bModel = false;
	Initd3dDevice();
	g_ClsModelObject.CreateModelObject(m_pd3dDevice);
	m_bModel = true;
	if (m_hWnd)
		GetClientRect(m_hWnd, &m_ClientRect);
	RenderScene();
}


void CMainProcess::SetViewport()
{
	RECT rect;

	gClsVisual.CreateLineObject(m_pd3dDevice);
	::GetClientRect(m_hWnd, &rect);
	m_ClientRect = rect;
	for(int i=0; i<3; i++)
	{
		m_bselected[i] = false;
	}
	m_bselected[3] = true;
	m_intViewMode = VIEW_ALL;
	m_intRenderMode = FULL_MODE;
	m_ClsLineProc.CreateLineObject(m_pd3dDevice);

	//CVisual3D* visual3D = new CVisual3D();
	//visual3D->CreateLineObject(m_pd3dDevice);
	for(int i=0; i<4; i++)
	{
		//m_ClsViewport[i].AddVisual3D(visual3D);
		m_ClsViewport[i].InitCamera(&m_StrCameras[i]);
		m_ClsViewport[i].CreateLineObject(m_pd3dDevice);
	}
	m_ClsViewport[0].SetDrawStrokes(true);
	m_ClsViewport[0].SetDrawText(true);
	m_ClsViewport[0].SetTitle(_T("내려보기"));
	m_ClsViewport[1].SetTitle(_T("정면보기"));
	m_ClsViewport[2].SetTitle(_T("측면보기"));
	m_ClsViewport[3].SetTitle(_T("립체보기"));

	m_ClsViewport[0].SetProjectMode(ORTHOGRAPHY);
	m_ClsViewport[1].SetProjectMode(ORTHOGRAPHY);
	m_ClsViewport[2].SetProjectMode(ORTHOGRAPHY);
	m_ClsViewport[3].SetProjectMode(PERSPECTIVE);
}

void CMainProcess::InitCamera()
{
	if (g_arrObjects.GetCount() < 1)
		return;

	float w,d,h;
	ObjectNode** ppNode =  g_arrObjects.GetItem(0);
	w =  ppNode[0]->w;//pModel->width;
	d = ppNode[0]->d;//pModel->depth;
	h = ppNode[0]->h;//pModel->height;
	float max = w*w+d*d+h*h;
	max = sqrt(max);
	/*if(d > max) max = d;
	if(h > max) max = h;*/
	D3DXVECTOR3 vCenter;
	vCenter.x = ppNode[0]->x + w/2;
	vCenter.y = ppNode[0]->y + d/2;
	vCenter.z = ppNode[0]->z + h/2;
	float width[3], height[3];
	width[0] = w;
	height[0] = d;
	width[1] = w;
	height[1] = h;
	width[2] = h;
	height[2] = d;
	for(int i=0; i<4; i++)
	{
		m_StrCameras[i].vLookAt = vCenter;
		m_StrCameras[i].vEyePos = vCenter;
		m_StrCameras[i].nearDist = max;
		m_StrCameras[i].farDist = 6*max;
		m_StrCameras[i].fDist = 2.0f*max;
		m_StrCameras[i].fMax = max;
		m_StrCameras[i].vCenter = vCenter;
		m_StrCameras[i].vObjectPos = vCenter;
		m_StrCameras[i].length = 1.5f*max;
		m_StrCameras[i].vUp = _vUp[i];
		m_StrCameras[i].fov = 25; 
	}
	m_StrCameras[0].vEyePos.z += 2.5f*max; 
	m_StrCameras[0].vLookAt.z -= 2.5f*max;
	m_StrCameras[1].vEyePos.y += 2.5f*max; 
	m_StrCameras[1].vLookAt.y -= 2.5f*max;
	m_StrCameras[2].vEyePos.x += 2.5f*max; 
	m_StrCameras[2].vLookAt.x -= 2.5f*max;
	m_StrCameras[3].vEyePos.y += 2.0f*max;
	m_StrCameras[3].vLookAt.y -= 2.0f*max;
	m_StrCameras[0].mode = ORTHOGRAPHY;
	m_StrCameras[1].mode = ORTHOGRAPHY;
	m_StrCameras[2].mode = ORTHOGRAPHY;
	m_StrCameras[3].mode = PERSPECTIVE;
}

void CMainProcess::RenderScene()
{
	RECT rects[8];

	if (m_hWnd == NULL)
		return;

	//Render all models
	if (!m_bModel)
		return;

	rects[0].left = m_ClientRect.left;
	rects[0].top = m_ClientRect.top;
	rects[0].right = m_ClientRect.right/2;
	rects[0].bottom = m_ClientRect.bottom/2;

	rects[1].left = m_ClientRect.right/2;
	rects[1].top = m_ClientRect.top;
	rects[1].right = m_ClientRect.right;
	rects[1].bottom = m_ClientRect.bottom/2;

	rects[2].left = m_ClientRect.left;
	rects[2].top = m_ClientRect.bottom/2;
	rects[2].right = m_ClientRect.right/2;
	rects[2].bottom = m_ClientRect.bottom;

	rects[3].left = m_ClientRect.right/2;
	rects[3].top = m_ClientRect.bottom/2;
	rects[3].right = m_ClientRect.right;
	rects[3].bottom = m_ClientRect.bottom;

	for(int i=0; i<4; i++)
	{
		m_ClsViewport[i].SetViewport(m_pd3dDevice,NULL);
	}

	switch(m_intViewMode)
	{
		case VIEW_ALL:
			for(int i=0; i<4; i++)
			{
				m_ClsViewport[i].Render(m_pd3dDevice,m_intRenderMode,m_bselected[i],&rects[i]);
			}
			break;
		case VIEW_TOP:
			m_ClsViewport[0].Render(m_pd3dDevice,m_intRenderMode,true,&m_ClientRect);
			break;
		case VIEW_FRONT:
			m_ClsViewport[1].Render(m_pd3dDevice,m_intRenderMode,true,&m_ClientRect);
			break;
		case VIEW_LEFT:
			m_ClsViewport[2].Render(m_pd3dDevice,m_intRenderMode,true,&m_ClientRect);
			break;
		case VIEW_PERSPECTIVE:
			m_ClsViewport[3].Render(m_pd3dDevice,m_intRenderMode,true,&m_ClientRect);
			break;
	}
}

void CMainProcess::SetViewMode(int mode)
{
	int oldView, oldRender;
	oldView = m_intViewMode;
	oldRender = m_intRenderMode;

	m_intViewMode = mode;
	if (mode == 0)
		mode = 1;
	
	m_intSelected = mode-1;
	if (mode == VIEW_PERSPECTIVE )
		m_intRenderMode = FULL_MODE;

	if ((oldView != m_intViewMode) || (oldRender != m_intRenderMode))
		RenderScene();
}

void CMainProcess::SetRenderMode(int mode)
{
	int old = m_intRenderMode;
	m_intRenderMode = mode;
	if (old != m_intRenderMode)
		RenderScene();
}

void CMainProcess::OnLButtonDown(POINT* pt)
{
	if (!m_bModel)
		return;
	RECT rect[4];
	if(m_intViewMode==VIEW_ALL)
	{
		for(int i=0; i<4; i++)
		{
			m_bselected[i] = false;
			m_ClsViewport[i].GetViewRect(&rect[i]);
			if(PtInRect(&rect[i],*pt))
			{
				m_bselected[i] = true;
				m_intSelected = i;//0 VIEW_ALL, 1 TOP, 2 FRONT, 3 LEFT, 4 PERSPECTIVE 
			}
		}
	}
	if(!m_bLineDrawing)
	{
		m_ClsHitProc.PickObject(pt->x,pt->y,&m_ClsViewport[m_intSelected],m_intSelected+1,m_pd3dDevice);
	}
	if (m_bLineDrawing)
	{
		m_ClsViewport[0].GetViewRect(&rect[0]);
		if(PtInRect(&rect[0],*pt))
			m_ClsLineProc.AddPoint(pt->x, pt->y, &m_ClsViewport[0], m_pd3dDevice);
	}
	m_bMousing = true;
	m_LastPt = *pt;

	RenderScene();
}

void CMainProcess::OnLButtonUp(POINT* pt)
{
	BOOL bResult = FALSE;

	if (m_bMousing)
	{
		m_ClsHitProc.LocatePickedObject();
		//m_ClsHitProc.WeldingObjects();
		bResult = TRUE;
	}
	m_bMousing = false;

	if (bResult)
		RenderScene();
}

void CMainProcess::OnMouseMove(POINT* pt)
{
	RECT rect;

	if (!m_bMousing)
		return;

	::GetClientRect(m_hWnd, &rect);
	if (::PtInRect(&rect, *pt)==FALSE)
	{
		m_bMousing = FALSE;
		return;
	}
	if (!m_bModel)
		return;

	//rotate object in perspective view 
	m_ClsViewport[3].GetViewRect(&rect);
	if(PtInRect(&rect,*pt) && m_intSelected == VIEW_PERSPECTIVE-1 )
	{
		float fSpinX = (float) (pt->x - m_LastPt.x);
		float fSpinY = (float) (pt->y - m_LastPt.y);
		m_ClsViewport[3].RotateCamera(fSpinX,fSpinY);
	}
	//move the picked object in top view
	m_ClsViewport[0].GetViewRect(&rect);
	if(!m_bLineDrawing)
	{
		if(PtInRect(&rect,*pt) && m_intSelected == VIEW_TOP-1)
		{
			m_ClsHitProc.MoveObject(&m_LastPt,pt,&m_ClsViewport[0],m_pd3dDevice);
		}
	}
	//draw line
	if (m_bLineDrawing && m_intSelected == VIEW_TOP-1)
	{
		if(PtInRect(&rect,*pt))
		{
			m_ClsLineProc.SetPoint(pt->x,pt->y,&m_ClsViewport[0],m_pd3dDevice);
		}
	}
	m_LastPt = *pt;

	RenderScene();
}

void CMainProcess::ScrollDown(POINT* pPt)
{
	RECT rect[4];
	POINT pt;
	pt.x = pPt->x;
	pt.y = pPt->y;
	D3DXVECTOR3 vPos, vDir;
	
	for(int i=0; i<4; i++)
	{
		m_ClsViewport[i].GetViewRect(&rect[i]);
		if(PtInRect(&rect[i],pt))
		{
			m_ClsViewport[i].MoveCamera(m_pd3dDevice);
			int mode = m_ClsViewport[i].GetProjectMode();
			switch(mode)
			{
			case PERSPECTIVE:
				m_ClsHitProc.GetRayDir(pt.x,pt.y,&vPos,&vDir,&m_ClsViewport[i],m_pd3dDevice);
				m_ClsViewport[i].ZoomOut(&vDir);
				break;
			case ORTHOGRAPHY:
				m_ClsHitProc.Orthographic_MouseToWorld(&vPos,pt.x,pt.y,&m_ClsViewport[i],m_pd3dDevice);
				m_ClsViewport[i].ZoomOut(&vPos);
				break;
			}
		}
	}

	RenderScene();
}

void CMainProcess::ScrollUp(POINT* pPt)
{
	RECT rect;
	POINT pt;
	pt.x = pPt->x;
	pt.y = pPt->y;
	D3DXVECTOR3 vPos, vDir;
	for(int i=0; i<4; i++)
	{
		m_ClsViewport[i].GetViewRect(&rect);
		if(PtInRect(&rect,pt))
		{
			m_ClsViewport[i].MoveCamera(m_pd3dDevice);
			int mode = m_ClsViewport[i].GetViewMode();
			switch(mode)
			{
			case PERSPECTIVE:
				m_ClsHitProc.GetRayDir(pt.x,pt.y,&vPos,&vDir,&m_ClsViewport[i],m_pd3dDevice);
				m_ClsViewport[i].ZoomIn(&vDir);
				break;
			case ORTHOGRAPHY:
				m_ClsHitProc.Orthographic_MouseToWorld(&vPos,pt.x,pt.y,&m_ClsViewport[i],m_pd3dDevice);
				m_ClsViewport[i].ZoomIn(&vPos);
				break;
			}
		}
	}

	RenderScene();
}

bool CMainProcess::SetTexture(int matId)
{
	bool	bResult = false;
	TCHAR	szPath[XML_MAX_PATH_LEN];
	LPXML_MATINFO	pMat = g_arrMats.GetItem(matId);
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);

	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if (ppNodes[i]->isSelected())
		{
			D3DXIMAGE_INFO srcInfo;
			PALETTEENTRY   palette;
			switch (pMat->nType)
			{
			case XML_MT_COLOR:
				if (pMat->pTexture)
				{
					pMat->pTexture->Release();
					pMat->pTexture = NULL;
				}
				ppNodes[i]->nMatIndex = matId;
				bResult = ppNodes[i]->setAloneColor();
				break;
			case XML_MT_IMAGE:
				GetImageAbsolutePath(szPath, pMat->szImagePath);
				if (pMat->pTexture==NULL)
				{
					if (D3DXCreateTextureFromFile(m_pd3dDevice, szPath, &(pMat->pTexture)) != D3D_OK)
						return false;
					char fileName[MAXCHAR];
					_copy_tchar2char(fileName, szPath);
					IplImage* lpImg = cvLoadImage(fileName);
					pMat->pColors = new D3DXCOLOR[lpImg->width*lpImg->height];
					pMat->width = lpImg->width;
					pMat->height = lpImg->height;
					CvScalar s;
					int l=0;
					for(int ix=0; ix<lpImg->height; ix++)
					{
						for(int iy=0; iy<lpImg->width; iy++)
						{
							s = cvGet2D(lpImg,ix,iy);
							pMat->pColors[l].b = s.val[0]/255;
							pMat->pColors[l].g = s.val[1]/255;
							pMat->pColors[l].r = s.val[2]/255;
							l++;
						}
					}
				}
				ppNodes[i]->nMatIndex = matId;
				bResult = true;
				break;
			}
		}
	}

	if (bResult)
		RenderScene();

	return bResult;
}

void CMainProcess::OnRButtonDown(POINT* pt)
{
	if (m_bLineDrawing)
	{
		m_ClsLineProc.AddLine();
		RenderScene();
	}
}

void CMainProcess::SetLineDrawing(BOOL bFlag)
{
	m_bLineDrawing = bFlag;
	RenderScene();
}

void CMainProcess::RotatePickedObject()
{
	if (m_intSelected==VIEW_TOP-1)
		m_ClsHitProc.RotateObject(D3DX_PI/2);
	RenderScene();
}

void CMainProcess::ScatterObjects(int nCount)
{
	float	s = 0.02f;
	if (!m_bModel)
		return;

	if (nCount < 0)
	{
		nCount = -nCount;
		s = -s;
	}

	while (nCount)
	{
		m_ClsHitProc.ScatteringObjects(s);
		nCount--;
	}
	RenderScene();
}

void CMainProcess::ReSize()
{
	RECT	rcOld;

	if (m_hWnd == NULL)
		return;

	//Get Old Client Rect
	rcOld = m_ClientRect;
	//Get Current Client Rect
	::GetClientRect(m_hWnd, &m_ClientRect);
	//Render Scene Objects
	if (((rcOld.right - rcOld.left) != (m_ClientRect.right - m_ClientRect.left)) || ((rcOld.bottom - rcOld.top) != (m_ClientRect.bottom - m_ClientRect.top)))
		RenderScene();
}

VOID CMainProcess::SetPointLight(LPDIRECT3DDEVICE9 d3dDevice)
{
	D3DLIGHT9 d3dLight[8];
	D3DXVECTOR3 vCenter;

	// Initialize the structure.
	ZeroMemory(&d3dLight, sizeof(d3dLight));

	// Set up a white point light.
	for(int i=4; i<8; i++)
	{
		d3dLight[i].Type = D3DLIGHT_POINT;
		d3dLight[i].Diffuse.r  = 0.35f;
		d3dLight[i].Diffuse.g  = 0.35f;
		d3dLight[i].Diffuse.b  = 0.35f;
		d3dLight[i].Ambient.r  = 0.0f;
		d3dLight[i].Ambient.g  = 0.0f;
		d3dLight[i].Ambient.b  = 0.0f;
		d3dLight[i].Specular.r = 1.0f;
		d3dLight[i].Specular.g = 0.0f;
		d3dLight[i].Specular.b = 0.0f;
		// Don't attenuate.
		d3dLight[i].Attenuation0 = 0.9f; 
		d3dLight[i].Range        = 16000.0f;
	}
	for(int i=0; i<4; i++)
	{
		d3dLight[i].Type = D3DLIGHT_POINT;
		d3dLight[i].Diffuse.r  = 0.2f;
		d3dLight[i].Diffuse.g  = 0.2f;
		d3dLight[i].Diffuse.b  = 0.2f;
		d3dLight[i].Ambient.r  = 0.0f;
		d3dLight[i].Ambient.g  = 0.0f;
		d3dLight[i].Ambient.b  = 0.0f;
		d3dLight[i].Specular.r = 1.0f;
		d3dLight[i].Specular.g = 0.0f;
		d3dLight[i].Specular.b = 0.0f;
		// Don't attenuate.
		d3dLight[i].Attenuation0 = 0.95f; 
		d3dLight[i].Range        = 16000.0f;
	}
	// Position it high in the scene and behind the user.
	// Remember, these coordinates are in world space, so
	// the user could be anywhere in world space, too. 
	// For the purposes of this example, assume the user
	// is at the origin of world space.
	
	if (g_arrObjects.GetCount() > 0)
	{
		ObjectNode** ppNodes = g_arrObjects.GetItem(0);
		vCenter.x = ppNodes[0]->x + ppNodes[0]->w/2;
		vCenter.y = ppNodes[0]->y + ppNodes[0]->d/2;
		vCenter.z = ppNodes[0]->z + ppNodes[0]->h/2;
		float fMax = ppNodes[0]->w;
		if(fMax < ppNodes[0]->h)
			fMax = ppNodes[0]->h;
		if(fMax < ppNodes[0]->d)
			fMax = ppNodes[0]->d;
		for(int i=0; i<8; i++)
		{
			d3dLight[i].Position = vCenter + _vLightPos[i];
			d3dDevice->SetLight(i, &d3dLight[i]);
			d3dDevice->LightEnable(i,TRUE);
		}
	}
}


void GetScreenResolution(LONG* pnWidth, LONG* pnHeight)
{
	HWND	hDesk;
	RECT	rcWnd = {0, 0};

	hDesk = GetDesktopWindow();
	GetWindowRect(hDesk, &rcWnd);
	rcWnd.right -= rcWnd.left;
	rcWnd.bottom-= rcWnd.top;
	if (pnWidth)
		*pnWidth = rcWnd.right;
	if (pnHeight)
		*pnHeight = rcWnd.bottom;
}
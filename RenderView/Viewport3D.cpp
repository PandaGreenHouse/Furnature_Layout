#include "stdafx.h"
#include "Model.h"
#include "Viewport3D.h"
#include "MyArray.h"
#include "math.h"
CMyArray<D3DXVECTOR3> g_ArrayVert3D;
LineNode g_LineNode;

CViewport3D::CViewport3D()
{
	m_intCount = 0;

	m_pLine = NULL;
	m_bStroke = false;

	m_pFont = NULL;
	m_bDrawText = false;

	m_pTitleFont = NULL;	
}

void CViewport3D::CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//Create Viewport Border Line
	if(m_pLine)
	{
		m_pLine->Release();
		m_pLine = NULL;
	}
	D3DXCreateLine(pd3dDevice, &m_pLine);
	m_pLine->SetWidth( 2.0f );
	m_pLine->SetAntialias(TRUE);
	m_pLine->SetGLLines(TRUE);

	//Create Font for Values of Objects
	if (m_pFont) 
	{
		m_pFont->Release();
		m_pFont = NULL;
	}

	D3DXCreateFont(pd3dDevice, 15, 0, FW_THIN, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"), &m_pFont);

	//Create Font for Viewport Title
	if (m_pTitleFont) 
	{
		m_pTitleFont->Release();
		m_pTitleFont = NULL;
	}
	D3DXCreateFont(pd3dDevice, 20, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
					DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("SimSun"), &m_pTitleFont);
}

void CViewport3D::Release()
{
	if(m_pLine)
	{
		m_pLine->Release();
		m_pLine = NULL;
	}

	if(m_pFont)
	{
		m_pFont->Release();
		m_pFont = NULL;
	}

	if (m_pTitleFont)
	{
		m_pTitleFont->Release();
		m_pTitleFont = NULL;
	}
	m_intCount = 0;
}

void CViewport3D::AddVisual3D(CVisual3D* pVisual3D)
{
	if (!pVisual3D)
		return;
	m_intCount++;
}

void CViewport3D::Delete(int intSelect)
{
	if (intSelect > m_intCount)
		return;

	m_intCount--;
}

void CViewport3D::GetViewRect(RECT* pRect)
{
	*pRect = m_ViewRect;
}

void CViewport3D::SetViewport(LPDIRECT3DDEVICE9 pd3dDevice, RECT* pRect)
{
	D3DVIEWPORT9 viewport;

	if (pRect==NULL)
	{
		m_ViewRect.left = 0;
		m_ViewRect.right = 0;
		m_ViewRect.top = 0;
		m_ViewRect.bottom = 0;
		return;
	}

	m_ViewRect = *pRect;
	viewport.X = pRect->left;
	viewport.Y = pRect->top;
	viewport.Width = pRect->right - pRect->left;
	viewport.Height = pRect->bottom - pRect->top;
	viewport.MaxZ = 1.0;
	viewport.MinZ = 0.0;
	pd3dDevice->SetViewport(&viewport);
	m_Center2D.x = (pRect->left + pRect->right)/2;
	m_Center2D.y = (pRect->top + pRect->bottom)/2;
}

void CViewport3D::Render(LPDIRECT3DDEVICE9 pd3dDevice, int intRendermode, bool bSelected, RECT* pRect)
{
	m_ViewRect = *pRect;
	D3DVIEWPORT9 viewport;
	viewport.X = pRect->left;
	viewport.Y = pRect->top;
	viewport.Width = pRect->right - pRect->left;
	viewport.Height = pRect->bottom - pRect->top;
	viewport.MaxZ = 1.0;
	viewport.MinZ = 0.0;
	pd3dDevice->SetViewport(&viewport);
	MoveCamera(pd3dDevice);
	gClsVisual.Render(pd3dDevice,intRendermode);
	DrawViewportLines( bSelected, pRect);
	if(m_bStroke)
		DrawStrokes(pd3dDevice);
	//if(m_bDrawText)
		//DrawText(pd3dDevice);
	DrawTitle();
	pd3dDevice->Present( &m_ViewRect, &m_ViewRect, NULL, NULL );
}

void CViewport3D::DrawViewportLines(bool bSelected, RECT* pRect)
{
	D3DXVECTOR2 vert[5];
	vert[0].x = 3;
	vert[0].y = 3;
	vert[1].x = 3;
	vert[1].y = (float) (pRect->bottom - pRect->top - 3);
	vert[2].x = (float) (pRect->right - pRect->left - 3);
	vert[2].y = (float) (pRect->bottom - pRect->top - 3);
	vert[3].x = (float) (pRect->right - pRect->left - 3);
	vert[3].y = 3;
	vert[4] = vert[0];
	if(bSelected==FALSE)
	{
		DWORD color = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,1.0);
		m_pLine->Begin();
		m_pLine->Draw(&vert[0],5,color);
		m_pLine->End();
	}
	else
	{
		DWORD color = D3DCOLOR_COLORVALUE(0.0,1.0,0.0,1.0);
		m_pLine->Begin();
		m_pLine->Draw(&vert[0],5,color);
		m_pLine->End();
	}
}

void CViewport3D::InitCamera(Camera* pCamera)
{
	memcpy(&m_StrCamera, pCamera, sizeof(Camera));
}

void GetScreenSize(int* pnWidth, int* pnHeight)
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

void CViewport3D::MoveCamera(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//AdjustCamera();
	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	pd3dDevice->SetTransform( D3DTS_VIEW, &matIdentity );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matIdentity );
	D3DXMATRIX matView;
	D3DXVec3Normalize(&m_StrCamera.vUp,&m_StrCamera.vUp);
	D3DXMatrixLookAtLH( &matView, &(m_StrCamera.vEyePos), &(m_StrCamera.vLookAt), &(m_StrCamera.vUp) );
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
	D3DXMATRIX matProj;
	float w = (float) (m_ViewRect.right - m_ViewRect.left);
	float h = (float) (m_ViewRect.bottom - m_ViewRect.top);
	m_StrCamera.aspect = w/h;
	if(m_StrCamera.mode==PERSPECTIVE)
	{
		/*int w = 0, h = 0;
		GetScreenSize(&w, &h);*/
		D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( m_StrCamera.fov ), m_StrCamera.aspect, m_StrCamera.nearDist, m_StrCamera.farDist);
		g_MyCameras[3].aspect = m_StrCamera.aspect;
		g_MyCameras[3].farDist = m_StrCamera.farDist;
		g_MyCameras[3].fov = D3DX_PI*m_StrCamera.fov/180.0f;
		g_MyCameras[3].nearDist = m_StrCamera.nearDist;
		g_MyCameras[3].vEyePos = m_StrCamera.vEyePos;
		g_MyCameras[3].vLookAt = m_StrCamera.vLookAt;
		g_MyCameras[3].vUp = m_StrCamera.vUp;
		g_MyCameras[3].vObjectPos = m_StrCamera.vObjectPos;
		g_MyCameras[3].fMax = m_StrCamera.fMax;
		//memccpy(&g_MyCameras[3],&m_StrCamera,0,sizeof(Camera));
	}
	else
	{
		float s = w/h;
		h = 1.5f*m_StrCamera.length;
		w = s*h;
		m_StrCamera.width = w;
		m_StrCamera.height = h;
		D3DXMatrixOrthoLH( &matProj,w,h,0.0f,m_StrCamera.farDist);//m_StrCamera.nearDist,m_StrCamera.farDist);
	}
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}

void CViewport3D::RotateCamera(float rotX, float rotY)
{
	D3DXMATRIX matRot[3];
	D3DXMatrixRotationAxis(&matRot[0],	&m_StrCamera.vUp, 0.5f*D3DXToRadian(rotX));
	D3DXVECTOR3 vLeft, vDir;
	D3DXVECTOR3 vUp = m_StrCamera.vUp; 
	vDir = m_StrCamera.vLookAt - m_StrCamera.vEyePos;
	D3DXVec3Normalize(&vDir,&vDir);
	D3DXVec3Cross(&vLeft,&m_StrCamera.vUp,&vDir);
	D3DXVec3Normalize(&vLeft,&vLeft);
	D3DXMatrixRotationAxis(&matRot[1],	&vLeft, 0.5f*D3DXToRadian(rotY));
	D3DXMatrixMultiply( &matRot[2], &matRot[0], &matRot[1]);
	D3DXVECTOR4 vOut;
	D3DXVec3Transform(&vOut,&m_StrCamera.vUp,&matRot[2]);
	m_StrCamera.vUp.x = vOut.x;
	m_StrCamera.vUp.y = vOut.y;
	m_StrCamera.vUp.z = vOut.z;
	/*rotate the eyepos and lookat around the origin of the object*/
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	D3DXVECTOR3 rotCenter = D3DXVECTOR3(ppNodes[0]->x,ppNodes[0]->y,ppNodes[0]->z); 
	D3DXVECTOR3 vEyePos, vLookAt;
	vEyePos = m_StrCamera.vEyePos - rotCenter;
	vLookAt = m_StrCamera.vLookAt - rotCenter;
	D3DXVec3Transform(&vOut,&vEyePos,&matRot[2]);
	vEyePos.x = vOut.x;
	vEyePos.y = vOut.y;
	vEyePos.z = vOut.z;
	m_StrCamera.vEyePos = vEyePos + rotCenter;
	D3DXVec3Transform(&vOut,&vLookAt,&matRot[2]);
	vLookAt.x = vOut.x;
	vLookAt.y = vOut.y;
	vLookAt.z = vOut.z;
	m_StrCamera.vLookAt = vLookAt + rotCenter;
}

void CViewport3D::GetCameraDir(D3DXVECTOR3* vDir)
{
	D3DXVECTOR3 camDir = m_StrCamera.vLookAt - m_StrCamera.vEyePos;
	D3DXVec3Normalize(&camDir,&camDir);
	*vDir = camDir;
}

void CViewport3D::ZoomIn(D3DXVECTOR3* v)
{
	switch (m_StrCamera.mode)
	{
	case PERSPECTIVE:
		PerspectiveZoomIn(v);
		break;
	case ORTHOGRAPHY:
		OrthographyZoomIn(v);
		break;
	}
}

void CViewport3D::ZoomOut(D3DXVECTOR3* v)
{
	switch (m_StrCamera.mode)
	{
	case PERSPECTIVE:
		PerspectiveZoomOut(v);
		break;
	case ORTHOGRAPHY:
		OrthographyZoomOut(v);
		break;
	}
}

void CViewport3D::PerspectiveZoomIn(D3DXVECTOR3* vDir)
{
	m_StrCamera.fov -= 0.5f;
	if(m_StrCamera.fov < 0.1f) 
	{
		m_StrCamera.fov = 0.1f;
		return;
	}
	D3DXVECTOR3 vLeft, vUp, vForward;
	vForward = m_StrCamera.vLookAt - m_StrCamera.vEyePos;
	D3DXVec3Normalize(&vForward,&vForward);
	vUp = m_StrCamera.vUp;
	D3DXVec3Cross(&vLeft,&vUp,&vForward);
	D3DXVec3Normalize(&vLeft,&vLeft);
	float s,t;
	s = 500.0f*D3DXVec3Dot(vDir,&vLeft);
	t = 500.0f*D3DXVec3Dot(vDir,&vUp);
	m_StrCamera.vLookAt += s*vLeft + t*vUp;
	m_StrCamera.vEyePos += s*vLeft + t*vUp;
	/*ObjectNode** ppNode = g_arrObjects.GetItem(0);
	D3DXVECTOR3 vCenter = D3DXVECTOR3(ppNode[0]->x,ppNode[0]->y,ppNode[0]->z);
	D3DXVECTOR3 vec3D = vCenter - m_StrCamera.vEyePos;
	m_StrCamera.fDist = D3DXVec3Length(&vec3D);
	//if(m_StrCamera.fDist < m_StrCamera.fMax)
		//return;
	m_StrCamera.vEyePos += SCROLL_DELTA**vDir;  
	m_StrCamera.vLookAt += SCROLL_DELTA**vDir;*/
}

void CViewport3D::PerspectiveZoomOut(D3DXVECTOR3* vDir)
{
	m_StrCamera.fov += 1.5f;
	D3DXVECTOR3 vLeft, vUp, vForward;
	vForward = m_StrCamera.vLookAt - m_StrCamera.vEyePos;
	D3DXVec3Normalize(&vForward,&vForward);
	vUp = m_StrCamera.vUp;
	D3DXVec3Cross(&vLeft,&vUp,&vForward);
	D3DXVec3Normalize(&vLeft,&vLeft);
	float s,t,delta;
	delta = m_StrCamera.fMax/2;
	s = delta*D3DXVec3Dot(vDir,&vLeft);
	t = delta*D3DXVec3Dot(vDir,&vUp);
	m_StrCamera.vLookAt -= s*vLeft + t*vUp;
	m_StrCamera.vEyePos -= s*vLeft + t*vUp;
	//m_StrCamera.vEyePos -= SCROLL_DELTA**vDir;  
	//m_StrCamera.vLookAt -= SCROLL_DELTA**vDir;  
}

void CViewport3D::OrthographyZoomIn(D3DXVECTOR3* pTargetPos)
{
	float fOldLen = m_StrCamera.length;
	m_StrCamera.length = m_StrCamera.length - SCROLL_DELTA;
	if(m_StrCamera.length < SCROLL_DELTA)
		m_StrCamera.length = SCROLL_DELTA;
	float s = m_StrCamera.length/fOldLen;
	D3DXVECTOR3 vDir = (1-s)*(*pTargetPos-m_StrCamera.vEyePos);
	m_StrCamera.vEyePos += vDir;
	m_StrCamera.vLookAt += vDir;
}

void CViewport3D::OrthographyZoomOut(D3DXVECTOR3* pTargetPos)
{
	float fOldLen = m_StrCamera.length;
	m_StrCamera.length = m_StrCamera.length + SCROLL_DELTA;
	float s = m_StrCamera.length/fOldLen;
	D3DXVECTOR3 vDir = (s-1)*(m_StrCamera.vEyePos - *pTargetPos);
	m_StrCamera.vEyePos += vDir;
	m_StrCamera.vLookAt += vDir;
}

void CViewport3D::ToScreenCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint)
{
	D3DXMATRIX matProj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMATRIX matView;
	pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMATRIX matViewProj	= matView * matProj;   
	D3DVIEWPORT9 viewport;
	pd3dDevice->GetViewport(&viewport);
	DWORD width = viewport.Width/2;
	DWORD height = viewport.Height/2;
	D3DXVECTOR3 vert = *inPoint;
	D3DXVec3TransformCoord( &vert, &vert, &matViewProj );
	*outPoint = D3DXVECTOR2( vert.x * width + width, -vert.y * height + height );
}

void CViewport3D::DrawStrokes(LPDIRECT3DDEVICE9 pd3dDevice)
{
	DWORD color = D3DCOLOR_COLORVALUE(0.0,1.0,0.0,1.0);
	D3DXVECTOR2* pVert2D = NULL;
	D3DXVECTOR3* pVert3D = NULL;

	if (m_pLine == NULL)
		return;
	
	if(g_ArrayVert3D.GetCount()>1)
	{
		int nCount = g_ArrayVert3D.GetCount();
		pVert2D = new D3DXVECTOR2[nCount];
		pVert3D = g_ArrayVert3D.GetItem(0);
		for(int i=0; i<nCount; i++)
		{
			ToScreenCoord( pd3dDevice, &pVert3D[i], &pVert2D[i]);
		}
		m_pLine->Begin();
		m_pLine->Draw(pVert2D,nCount,color);
		m_pLine->End();
		delete[] pVert2D;
	}

	if(g_LineNode.nCount > 0)
	{
		LineNode* node = &g_LineNode;
		while(node)
		{
			int nCount = node->nCount;
			pVert2D = new D3DXVECTOR2[nCount];
			for(int i=0; i<nCount; i++)
			{
				ToScreenCoord( pd3dDevice, &node->pVertices[i], &pVert2D[i]);
			}
			m_pLine->Begin();
			m_pLine->Draw(pVert2D,nCount,color);
			m_pLine->End();
			delete[] pVert2D;
			node = node->next;
		}
	}
}

void CViewport3D::DrawText(LPDIRECT3DDEVICE9 pd3dDevice)
{
	TCHAR str[20];
	D3DXVECTOR3 vCenter;// = g_pStrChestModel->vCenter;
	D3DXVECTOR3 vPos3D[3]; 
	D3DXVECTOR2 vPos2D[3];

	if (m_pFont == NULL)
		return;

	ObjectNode** ppNode = g_arrObjects.GetItem(0);
	int width	= (int) ppNode[0]->w;//g_pStrChestModel->width;
	int height	= (int) ppNode[0]->h;//g_pStrChestModel->height;
	int depth	= (int) ppNode[0]->d;//g_pStrChestModel->depth;

	vCenter.x = ppNode[0]->x + width/2;
	vCenter.y = ppNode[0]->y + depth/2;
	vCenter.z = ppNode[0]->z + height/2;
	for(int i=0; i<3; i++)
	{
		vPos3D[i] = vCenter;
	}
	vPos3D[0].y -= depth/2; 
	vPos3D[1].x -= width/2; 
	vPos3D[2].y += depth/2; 
	float w = (float) (m_ViewRect.right - m_ViewRect.left)/2.0f;
	float h = (float) (m_ViewRect.bottom - m_ViewRect.top)/2.0f;
	for(int i=0; i<3; i++)
	{
		ToScreenCoord(pd3dDevice,&vPos3D[i],&vPos2D[i]);
	}

	int vals[3];
	RECT strReg[3];
	vals[0] = 2*width*height+2*depth*height+2*width*depth; 
	vals[1] = width;
	vals[2] = depth;
	vPos2D[0].y -= 20;
	vPos2D[1].x -= 30;
	for(int i=0; i<3; i++)
	{
		strReg[i].left	= (LONG) vPos2D[i].x;
		strReg[i].top	= (LONG) vPos2D[i].y;
		strReg[i].right	= (LONG) vPos2D[i].x + 60;
		strReg[i].bottom= (LONG) vPos2D[i].y + 30;
		_stprintf(str, _T("%d"),vals[i]);
		m_pFont->DrawText(NULL,str,-1,&strReg[i],DT_VCENTER,D3DCOLOR_ARGB(210,10,10,0));
	}
}

void CViewport3D::DrawTitle()
{
	RECT rect;
	rect.left = 10 + m_ViewRect.left;
	rect.top = 10 + m_ViewRect.top;
	rect.right = 100 + rect.left;
	rect.bottom = 50 + rect.top;
	if (m_pTitleFont)
		m_pTitleFont->DrawText(NULL,m_szTitle,-1,&rect,DT_LEFT,D3DCOLOR_ARGB(210,10,10,0));
}

void CViewport3D::RotateObject(float fSpinX, float fSpinY, float fSpinZ)
{
	D3DXQUATERNION qRotation[3];
	qRotation[0].x = 1.0f;
	qRotation[0].y = 0.0f;
	qRotation[0].z = 0.0f;
	qRotation[0].w = fSpinX;
	qRotation[1].x = 0.0f;
	qRotation[1].y = 1.0f;
	qRotation[1].z = 0.0f;
	qRotation[1].w = fSpinY;
	qRotation[2].x = 0.0f;
	qRotation[2].y = 0.0f;
	qRotation[2].z = 1.0f;
	qRotation[2].w = fSpinZ;
	m_StrRtTranform.QRotation = qRotation[0] + qRotation[1] + qRotation[2];
}

void CViewport3D::MoveObject(D3DXVECTOR3* vTranslation)
{
	m_StrRtTranform.vTranslation = *vTranslation;
}

void CViewport3D::ScalingObject(D3DXVECTOR3* vScaleCenter, float fScaleX, float fScaleY, float fScaleZ)
{
	m_StrRtTranform.vScaleCenter = *vScaleCenter;
	m_StrRtTranform.vScaling.x = fScaleX;
	m_StrRtTranform.vScaling.y = fScaleY;
	m_StrRtTranform.vScaling.z = fScaleZ;
}

void CViewport3D::AdjustCamera()
{
//	ChestModel* pChest = m_pFirst->pClsVisual->GetChestModel();
	ObjectNode** ppNode = g_arrObjects.GetItem(0);
	D3DXVECTOR3 vEyePos = m_StrCamera.vEyePos;
	D3DXVECTOR3 vForward =  m_StrCamera.vLookAt - m_StrCamera.vEyePos;
	D3DXVec3Normalize(&vForward,&vForward);
	float length;
	float fMin = FLT_MAX;
	float fMax = FLT_MIN;
	D3DXVECTOR3 vec3D;
	for(int i=0; i<6; i++)
	{
		for(int j=0; j<4; j++)
		{
			vec3D = ppNode[0]->vBounds[i][j] - vEyePos;
			length = D3DXVec3Dot(&vForward,&vec3D);
			length = abs(length);
			if(length < fMin)
				fMin = length;
			if(length > fMax)
				fMax = length; 
		}
	}
	m_StrCamera.nearDist = fMin/2;
	m_StrCamera.farDist = fMin + 4*m_StrCamera.fMax;
	D3DXVECTOR3 vCenter = D3DXVECTOR3(ppNode[0]->x+ppNode[0]->w/2,ppNode[0]->y+ppNode[0]->d/2,ppNode[0]->z+ppNode[0]->h/2);
	vec3D = m_StrCamera.vCenter - m_StrCamera.vEyePos;
	m_StrCamera.fDist = D3DXVec3Length(&vec3D);
}
#include "stdafx.h"
#include "LineProcess.h"

CLineProcess::CLineProcess(void)
{
	m_intPtCount = 0;
	m_intLineCount = 0;
	g_LineNode.nCount = 0;
	g_LineNode.next = NULL;
	g_LineNode.pVertices = NULL;
}

CLineProcess::~CLineProcess()
{
	if(g_LineNode.pVertices)
		delete[] g_LineNode.pVertices;
	LineNode* node = g_LineNode.next;
	while(node)
	{
		delete[] node->pVertices;
		node = node->next;
	}
	delete[] node;
}

void CLineProcess::CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(m_pLine) m_pLine->Release();
	D3DXCreateLine(pd3dDevice, &m_pLine);
	m_pLine->SetWidth( 2.0f );
	m_pLine->SetAntialias(TRUE);
	m_pLine->SetGLLines(TRUE);
}

void CLineProcess::AddPoint(int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_bAdded = false;
	D3DXVECTOR3 vert3D;
	m_ClsHitProc.Orthographic_MouseToWorld(&vert3D,x,y,viewport,pd3dDevice);
	if(g_ArrayVert3D.GetCount()==0)
	{
		g_ArrayVert3D.Clear();
		g_ArrayVert3D.Create(20,10);
		g_ArrayVert3D.AddItem(&vert3D);
		return;
	}
	D3DXVECTOR3* pVert = g_ArrayVert3D.GetLast();
	*pVert = vert3D;
}

void CLineProcess::SetPoint(int x, int y, CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice)
{
	D3DXVECTOR3 vert3D;
	m_ClsHitProc.Orthographic_MouseToWorld(&vert3D,x,y,viewport,pd3dDevice);
	if(!m_bAdded)
	{
		g_ArrayVert3D.AddItem(&vert3D);
		m_bAdded = true;
	}
	else
	{
		D3DXVECTOR3* pVert = g_ArrayVert3D.GetLast();
		*pVert = vert3D;
	}
}

void CLineProcess::AddLine()
{
	if(g_ArrayVert3D.GetCount()>1)
	{
		if(m_intLineCount == 0)
		{
			int nCount = g_ArrayVert3D.GetCount();
			g_LineNode.nCount = nCount;
			D3DXVECTOR3* pVert3D = g_ArrayVert3D.GetItem(0);
			g_LineNode.pVertices = new D3DXVECTOR3[nCount];
			for(int i=0; i<nCount; i++)
			{
				g_LineNode.pVertices[i] = pVert3D[i];
			}
			g_LineNode.next = NULL;
		}
		else
		{
			LineNode* Node = &g_LineNode;
			LineNode* prevNode; 
			while(Node)
			{
				prevNode = Node;
				Node = Node->next;
			}
			Node = new LineNode;
			Node->next = NULL;
			int nCount = g_ArrayVert3D.GetCount();
			Node->nCount = nCount;
			D3DXVECTOR3* pVert3D = g_ArrayVert3D.GetItem(0);
			Node->pVertices = new D3DXVECTOR3[nCount];
			for(int i=0; i<nCount; i++)
			{
				Node->pVertices[i] = pVert3D[i];
			}
			prevNode->next = Node; 
		}
		m_intLineCount++;
	}
	g_ArrayVert3D.Clear();
}

void CLineProcess::ToScreenCoord(CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint)
{
	viewport->MoveCamera(pd3dDevice);
	D3DXMATRIX matProj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMATRIX matView;
	pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	D3DXMATRIX matViewProj	= matView * matProj;   
	RECT rect;
	viewport->GetViewRect(&rect);
	DWORD width = rect.right - rect.left;
	DWORD height = rect.bottom - rect.top;
	D3DXVECTOR3 vert = *inPoint;
	D3DXVec3TransformCoord( &vert, &vert, &matViewProj );
	*outPoint = D3DXVECTOR2( vert.x * width + width, -vert.y * height + height );
}

void CLineProcess::MouseToProject(CViewport3D* viewport, LPDIRECT3DDEVICE9 pd3dDevice, POINT* pt, D3DXVECTOR2* vert2D)
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
	x = (float) (pt->x - rect.left);
	y = (float) (pt->y - rect.top);
	px = ((( 2.0f * x) / width)  - 1.0f) / proj(0, 0);
	py = (((-2.0f * y) / height) + 1.0f) / proj(1, 1);
	vert2D->x = px;
	vert2D->y = py;
}

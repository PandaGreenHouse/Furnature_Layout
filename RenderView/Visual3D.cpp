#include "stdafx.h"
#include "Visual3D.h"
#include "Model.h"
#include <math.h>

CVisual3D::CVisual3D(void)
{
	m_pLine = NULL;
	m_vScalingCenter = D3DXVECTOR3(0.0f,0.0f,0.0f);
	m_vScaling = D3DXVECTOR3(1.0f,1.0f,1.0f);
}

void CVisual3D::Release()
{
	if (m_pLine)
	{
		m_pLine->Release();
		m_pLine = NULL;
	}
}

void CVisual3D::Render(LPDIRECT3DDEVICE9 pd3dDevice, int mode)
{
	switch(mode)
	{
	case FULL_MODE:
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
					D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f), 1.0f, 0 );
		FullRender(pd3dDevice);
		break;
	case WIRE_MODE:
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
					D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f), 1.0f, 0 );
		WireframeRender(pd3dDevice);
		break;
	case SHAPE_MODE:
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
					D3DCOLOR_COLORVALUE(1.0f,1.0f,1.0f,1.0f), 1.0f, 0 );
		ShapeLineRender(pd3dDevice);
		break;
	}
	RenderPickedPlanks(pd3dDevice);
}

void CVisual3D::FullRender(LPDIRECT3DDEVICE9 pd3dDevice)
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		int nObjectCount = g_arrObjects.GetCount();
		ObjectNode** ppNodes = g_arrObjects.GetItem(0);
		int matId;
		for(int i=0; i<nObjectCount; i++)
		{
			//if(ppNodes[i]->nType!=XML_OT_PLANE)
			//	continue;
			if(!ppNodes[i]->pVertexBuffer)
				continue;
			if(!ppNodes[i]->pIndexBuffer)
				continue;
			matId = ppNodes[i]->nMatIndex;
			if (matId!=-1)
			{
				if (g_arrMats[matId].pTexture)
					pd3dDevice->SetTexture(0,g_arrMats[matId].pTexture);
				else
					pd3dDevice->SetTexture(0, NULL);
			} else
				pd3dDevice->SetTexture(0,NULL);
			pd3dDevice->SetStreamSource( 0, ppNodes[i]->pVertexBuffer, 0, sizeof(VERTEX) );
			pd3dDevice->SetFVF(D3DFVF_MY_VERTEX);
			pd3dDevice->SetIndices( ppNodes[i]->pIndexBuffer );
			pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, // PrimitiveType
				0,                  // BaseVertexIndex
				0,                  // MinIndex
				ppNodes[i]->intVtCount,                  // NumVertices
				0,                  // StartIndex
				ppNodes[i]->intFaceCount );  
		}
		pd3dDevice->EndScene();
	}
}

void CVisual3D::WireframeRender(LPDIRECT3DDEVICE9 pd3dDevice)
{
	//RenderShape(pd3dDevice);
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	ObjectNode** ppNode = g_arrObjects.GetItem(0);
	DWORD color = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,1.0);
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		for(int i=0; i<g_arrObjects.GetCount(); i++)
		{
			DrawObject3DLine(pd3dDevice,ppNode[i],color);
			//DrawObject2DLine(pd3dDevice,ppNode[i],color);
		}
		pd3dDevice->EndScene();
	}
	
	/*D3DXVECTOR2 vert2D[5];
	DWORD color = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,1.0);
	RenderShape(pd3dDevice);
	ObjectNode** ppNode = g_arrObjects.GetItem(0);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if(ppNode[i]->nType!=XML_OT_PLANE) continue;
		for(int j=0; j<6; j++)
		{
			for(int k=0; k<4; k++)
			{
				ToScreenCoord( pd3dDevice, &ppNode[i]->vBounds[j][k], &vert2D[k]);
			}
			vert2D[4] = vert2D[0];
			m_pLine->Begin();
			m_pLine->Draw(&vert2D[0],5,color);
			m_pLine->End();
		}
	}*/
}


void CVisual3D::GetMatrixToProjectCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX* pMat)
{
	D3DXMATRIX matProj;
	pd3dDevice->GetTransform(D3DTS_PROJECTION, &matProj);
	D3DXMATRIX matView;
	pd3dDevice->GetTransform(D3DTS_VIEW, &matView);
	*pMat = matView * matProj;   
}

void CVisual3D::ToScreenCoord(LPDIRECT3DDEVICE9 pd3dDevice, D3DXVECTOR3* inPoint, D3DXVECTOR2* outPoint)
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

void CVisual3D::RenderShape(LPDIRECT3DDEVICE9 pd3dDevice)
{
	ObjectNode** ppNodes = g_arrObjects.GetItem();
	int nObjectCount = g_arrObjects.GetCount();
	for(int i=0; i<nObjectCount; i++)
	{
		if(ppNodes[i]->pVertexBuffer==NULL) continue;
		pd3dDevice->SetTexture(0,NULL);
		if( SUCCEEDED( pd3dDevice->BeginScene() ) )
		{
			pd3dDevice->SetStreamSource( 0, ppNodes[i]->pVertexBuffer, 0, sizeof(VERTEX) );
			pd3dDevice->SetFVF(D3DFVF_MY_VERTEX);
			pd3dDevice->SetIndices( ppNodes[i]->pIndexBuffer );
			pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, // PrimitiveType
				0,                  // BaseVertexIndex
				0,                  // MinIndex
				ppNodes[i]->intVtCount,     // NumVertices
				0,                  // StartIndex
				ppNodes[i]->intFaceCount );  
		}
		pd3dDevice->EndScene();
	}
}

void CVisual3D::RenderPickedPlanks(LPDIRECT3DDEVICE9 pd3dDevice)
{
	/*DWORD color = D3DCOLOR_COLORVALUE(1.0,0.0,0.0,1.0);
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if(ppNodes[i]->isSelected() == false) continue;
		DrawObject3DLine(pd3dDevice,ppNodes[i],color);
	}*/
	if (m_pLine == NULL)
		return;

	D3DXVECTOR2 vert2D[5];
	DWORD color = D3DCOLOR_COLORVALUE(1.0,0.0,0.0,1.0);
	//RenderShape(pd3dDevice);
	ObjectNode** ppNode = g_arrObjects.GetItem(0);
	byte* pBytes = NULL;
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		if(ppNode[i]->isSelected() == false) continue; 
		//DrawObject2DLine(pd3dDevice, ppNode[i], color);
		//if(ppNode[i]->pLineVertexBuffer==NULL) continue;
		if(ppNode[i]->nType!=XML_OT_PLANE) continue;
		for(int j=0; j<6; j++)
		{
			for(int k=0; k<4; k++)
			{
				ToScreenCoord( pd3dDevice, &ppNode[i]->vBounds[j][k], &vert2D[k]);
			}
			vert2D[4] = vert2D[0];
			m_pLine->Begin();
			m_pLine->Draw(&vert2D[0],5,color);
			m_pLine->End();
		}
	}
}

void CVisual3D::CreateLineObject(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if (m_pLine) m_pLine->Release();
	D3DXCreateLine(pd3dDevice, &m_pLine);
	m_pLine->SetWidth( 1.0f );
	m_pLine->SetAntialias(TRUE);
	m_pLine->SetGLLines(TRUE);
}

void CVisual3D::ShapeLineRender(LPDIRECT3DDEVICE9 pd3dDevice)
{
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	DWORD color = D3DCOLOR_COLORVALUE(0.0,0.0,0.0,1.0);
	RenderShape(pd3dDevice);
	ObjectNode** ppNodes = g_arrObjects.GetItem(0);
	for(int i=0; i<g_arrObjects.GetCount(); i++)
	{
		DrawObject3DLine(pd3dDevice,ppNodes[i],color);
	}
}



void CVisual3D::DrawObject2DLine(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, DWORD color)
{
	int nLines = 0;
	LineVertex* pVertices;
	int* pIndices;
	//if(pNode->nType!=XML_OT_PLANE) return;
	if(pNode->pLineVertexBuffer==NULL) return;
	if( (pNode->pLineVertexBuffer)->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD ) != D3D_OK ) return;
	if(pNode->pLineIndexBuffer->Lock(0, 0, (void**)&pIndices, D3DLOCK_DISCARD)!=D3D_OK)	 return; 
	D3DXVECTOR2* vec2 = new D3DXVECTOR2[pNode->intLineVtCount];
	D3DXVECTOR2* vert2D = new D3DXVECTOR2[2*pNode->intLineCount];
	for(int j=0; j<pNode->intLineVtCount; j++)
	{
		ToScreenCoord( pd3dDevice, &pVertices[j].vertex, &vec2[j]);
	}
	for(int j=0; j<2*pNode->intLineCount; j++)
	{
		vert2D[j] = vec2[pIndices[j]];
	}
	pNode->pLineVertexBuffer->Unlock();
	pNode->pLineIndexBuffer->Unlock();
	m_pLine->Begin();
	m_pLine->Draw(&vert2D[0],2*pNode->intLineCount,color);
	m_pLine->End();
	delete[] vec2;
	delete[] vert2D;
}

#ifdef _MANAGED
#pragma managed(push, off)
#endif
void CVisual3D::DrawObject3DLine(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, DWORD color)
{
	//if(pNode->nType == XML_OT_HOLE) return;
	if(pNode->pLineVertexBuffer==NULL) return;
	LineVertex* pVertices;
	if( (pNode->pLineVertexBuffer)->Lock( 0, 0, (void**)&pVertices, D3DLOCK_DISCARD ) != D3D_OK ) return;
	for(int i=0; i<pNode->intLineVtCount; i++)
	{
		pVertices[i].diffuseColor = color;
	}
	(pNode->pLineVertexBuffer)->Unlock();
	//if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	//{
		pd3dDevice->SetStreamSource( 0, pNode->pLineVertexBuffer, 0, sizeof(LineVertex) );
		pd3dDevice->SetFVF(LINE_VERTEX);
		pd3dDevice->SetIndices( pNode->pLineIndexBuffer );
		if(pd3dDevice->DrawIndexedPrimitive( D3DPT_LINELIST, // PrimitiveType
			0,                  // BaseVertexIndex
			0,                  // MinIndex
			pNode->intLineVtCount, // NumVertices
			0,                  // StartIndex
			pNode->intLineCount )!=D3D_OK)
			return;
		//pd3dDevice->EndScene();
	//}
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

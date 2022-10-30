#include "stdafx.h"
#include "RayRender.h"
#include "MyArray.h"
#include "math.h"
#include "Model.h"
Camera		g_MyCameras[4];
LightSource		g_stuLight;
//extern CornellBox		g_stuCornellBox;
extern CBSPTree				g_ClsTree;
#define  MAX_COUNT	2		
#define  back_color		D3DXCOLOR(0.0f,0.0f,1.0f,0.0f)

CRayRender::CRayRender(void)
{
	m_pColors = NULL;
	m_nShadeType = 2;
}


CRayRender::~CRayRender(void)
{
	delete[] m_pColors;
}

void CRayRender::SetPointLight()
{
	float fMax = g_MyCameras[3].fMax;
	g_stuLight.fMaxDist = 2*fMax; 
	g_stuLight.vPos = D3DXVECTOR3(-fMax,-2.0f*fMax,4.0f*fMax);
	g_stuLight.fPower = 2*g_stuLight.fMaxDist;
}

void CRayRender::RT_RenderScene(HDC hDC, int Nx, INT Ny, BOOL bRender)
{
	if(g_arrObjects.GetCount()==0) return;
	SetPointLight();
	//시선광선발생
	D3DXVECTOR3 vOrigin, vLeft, vDir, vUp, vLookAt;
	vDir = g_MyCameras[3].vLookAt - g_MyCameras[3].vEyePos;
	D3DXVec3Normalize(&vDir,&vDir);
	vUp = g_MyCameras[3].vUp;
	D3DXVec3Cross(&vLeft,&vDir,&vUp);
	D3DXVec3Normalize(&vLeft,&vLeft);
	float Dx, Dy, dx, dy;
	Dy = 2.0f*tan(g_MyCameras[3].fov/2)*g_MyCameras[3].nearDist;
	Dx = Dy*g_MyCameras[3].aspect;
	int nDestWidth, nDestHeight;
	nDestWidth = Nx;//g_stuCamera.Nx;
	nDestHeight = Ny;//g_stuCamera.Ny;
	if(!bRender)
	{
		if(m_pColors)
			DrawPixels( hDC, m_pColors, nDestWidth, nDestHeight);
		return;
	}
	dx = Dx/nDestWidth;
	dy = Dy/nDestHeight;
	vOrigin = g_MyCameras[3].vEyePos + g_MyCameras[3].nearDist*vDir + 0.5f*Dx*vLeft + 0.5f*Dy*vUp;
	Ray ray;
	ray.vRayPos = g_MyCameras[3].vEyePos;
	if(m_pColors) free(m_pColors);
	m_pColors = (LPD3DXCOLOR)malloc(nDestWidth*nDestHeight*sizeof(D3DXCOLOR));
	DrawPixels( hDC, m_pColors, nDestWidth, nDestHeight);
	D3DXCOLOR color;
	DWORD l=0;
	static int nx=0,ny=0;
	for(int iy = 0; iy<nDestHeight; iy++)
	{
		for(int ix=0; ix<nDestWidth; ix++)
		{
			vLookAt = vOrigin - ix*dx*vLeft - iy*dy*vUp;
			vDir = vLookAt - ray.vRayPos;
			D3DXVec3Normalize(&vDir,&vDir);
			ray.vDir = vDir;
			ray.nObjectId = -1;
			ray.nFaceIndex = -1;
			ray.nCount = 0;
			ray.bLine = FALSE;
			color = RayTracing(&ray);
			SetPixel(hDC,ix,iy,RGB(255*color.r,255*color.g,255*color.b));
			m_pColors[l] = color;
			l++;
		}
	}
//	Smoothing(pColors,nDestWidth,nDestHeight);
//	DrawPixels( hDC, pColors, nDestWidth, nDestHeight);
}

void CRayRender::Smoothing(D3DXCOLOR* pColors, int width, int height)
{
	DWORD l=0;
	D3DXCOLOR colors[9];
	for(int iy = 1; iy<height-1; iy++)
	{
		for(int ix=1; ix<width-1; ix++)
		{
			l=ix+iy*width;
			colors[0] = pColors[l];
			l=ix+iy*width+1;
			colors[1] = pColors[l];
			l=ix+(iy-1)*width+1;
			colors[2] = pColors[l];
			l=ix+(iy-1)*width;
			colors[3] = pColors[l];
			l=ix+(iy-1)*width-1;
			colors[4] = pColors[l];
			l=ix+iy*width-1;
			colors[5] = pColors[l];
			l=ix+(iy+1)*width-1;
			colors[6] = pColors[l];
			l=ix+(iy+1)*width;
			colors[7] = pColors[l];
			l=ix+(iy+1)*width+1;
			colors[8] = pColors[l];
			for(int i=1; i<9; i++)
			{
				colors[0]+=colors[i];
			}
			colors[0]/=9;
			l=ix+iy*width;
			pColors[l] = colors[0];
		}
	}
}

void CRayRender::DrawPixels(HDC hDC, LPD3DXCOLOR pColors, int width, int height)
{
	DWORD l=0;
	char* pData = (char*)malloc(3*width*height);
	for(int iy=0; iy<height; iy++)
	{
		for(int ix=0; ix<width; ix++)
		{
			l = ix+iy*width;
			SetPixel(hDC,ix,iy,RGB(255*pColors[l].r,255*pColors[l].g,255*pColors[l].b));
		}
	}
}

BOOL CRayRender::GetReflectedRay(Ray* pReflectedRay, Ray* InRay)
{
	int faceId = InRay->nFaceIndex;
	int objId = InRay->nObjectId;
	D3DXVECTOR3 vNormal = D3DXVECTOR3(0.0f,0.0f,0.0f);
	VERTEX* pVertices;
	int* pIndices;
	LockBuffersOfObject(&pVertices,&pIndices,objId);
	for(int i=0; i<3; i++)
	{
		vNormal += pVertices[pIndices[3*faceId+i]].Normal;
	}
	UnlockBuffersOfObject(objId);
	D3DXVec3Normalize(&vNormal,&vNormal);
	D3DXVECTOR3 vDir = -InRay->vDir;
	D3DXVec3Normalize(&vDir,&vDir);
	float s = D3DXVec3Dot(&vNormal,&vDir);
	pReflectedRay->vDir = 2*s*vNormal - vDir;
	D3DXVec3Normalize(&pReflectedRay->vDir,&pReflectedRay->vDir);
	pReflectedRay->vRayPos = InRay->vIntersection + 0.01f*pReflectedRay->vDir;
	pReflectedRay->nObjectId = InRay->nObjectId;
	pReflectedRay->nFaceIndex = InRay->nFaceIndex;
	return TRUE;
}

float CRayRender::PhongShading(Ray* pRay)
{
	float cx[3];//가로축(x축)상에 놓이는 정점들의 x자리표. 0,1은 변과 x축의 사귐점, 2는 광선사귐점
	D3DXVECTOR3 vNormals[3];//
	D3DXVECTOR3 vxs[5], vxNormals[5];
	/*정점들에서 세기계산,정점자리표들을 카메라자리표계에로 변환*/ 
	D3DXMATRIX mat;
	D3DXMatrixLookAtLH(&mat, &g_MyCameras[3].vEyePos, &g_MyCameras[3].vLookAt, &g_MyCameras[3].vUp);
	D3DXVECTOR3 vLookAt = pRay->vRayPos + pRay->vDir;
	D3DXVECTOR3 vLeft,vUp;
	D3DXVec3Cross(&vLeft,&pRay->vDir,&g_MyCameras[3].vUp);
	D3DXVec3Normalize(&vLeft,&vLeft);
	D3DXVec3Cross(&vUp,&vLeft,&pRay->vDir);
	D3DXVec3Normalize(&vUp,&vUp);
	D3DXMatrixLookAtLH(&mat, &pRay->vRayPos, &vLookAt, &vUp);
	D3DXVECTOR4 out;
	D3DXVECTOR3 vert3D;
	if(pRay->bLine)
	{
		for(int i=0; i<4; i++)
		{
			vxs[i] = pRay->vxs[i];
			vxNormals[i] = pRay->vNormals[i];
		}
	}
	else
	{
		VERTEX* pVertices;
		int* pIndices;
		LockBuffersOfObject(&pVertices,&pIndices,pRay->nObjectId);
		int nFaceId = pRay->nFaceIndex;
		for(int i=0; i<3; i++)
		{
			vert3D = pVertices[pIndices[3*nFaceId+i]].Point;
			vxNormals[i] = -pVertices[pIndices[3*nFaceId+i]].Normal;
			D3DXVec3Transform(&out,&vert3D,&mat);
			vxs[i] = (D3DXVECTOR3)out;
			pRay->vxs[i] = vxs[i];
			pRay->vNormals[i] = vxNormals[i];
		}
		UnlockBuffersOfObject(pRay->nObjectId);
		vxs[3] = vxs[0];
		vxNormals[3] = vxNormals[0];
		pRay->vxs[3] = vxs[3];
		pRay->vNormals[3] = vxNormals[3];
	}
	D3DXVec3Transform(&out,&pRay->vIntersection,&mat);
	vxs[4] = (D3DXVECTOR3)out;
	/*변환완료*/
	/*phong방법으로 세기계산*/
	int k=0;
	for(int i=0; i<3; i++)
	{
		if(k==2) break;
		if((vxs[i].y < vxs[4].y && vxs[4].y < vxs[i+1].y) || (vxs[i].y > vxs[4].y && vxs[4].y > vxs[i+1].y))
		{
			float x[3],y[3],dx,dy[3];
			x[0] = vxs[i].x;	y[0] = vxs[i].y;	
			x[1] = vxs[i+1].x;	y[1] = vxs[i+1].y;
			x[2] = vxs[4].x;	y[2] = vxs[4].y;
			dx = x[0] - x[1];
			dy[0] = y[0] - y[1];
			dy[1] = y[0] - y[2];
			dy[2] = y[2] - y[1];
			//rad[k] = fabs((y[0]-y[2])/(y[0]-y[1]))*I[i+1] + fabs((y[2]-y[1])/(y[0]-y[1]))*I[i];
			//x[2] = (x[0]-x[1])*((y[2]-y[1])/(y[0]-y[1])+x[1]/(x[0]-x[1]));
			vNormals[k] = fabs(dy[1]/dy[0])*vxNormals[i+1] + fabs(dy[2]/dy[0])*vxNormals[i];
			D3DXVec3Normalize(&vNormals[k],&vNormals[k]);
			cx[k] = dx*dy[2]/dy[0] + x[1];
			k++;
		}
	}
	cx[2] = vxs[4].x;
	float l[3];
	l[0] = fabs(cx[2] - cx[0]);
	l[1] = fabs(cx[1] - cx[2]);
	l[2] = fabs(cx[1] - cx[0]);
	vNormals[2] = vNormals[0]*l[1]/l[2] + vNormals[1]*l[0]/l[2];
	D3DXVec3Normalize(&vNormals[2],&vNormals[2]);
	D3DXVECTOR3 vDir = g_stuLight.vPos - pRay->vIntersection;
	D3DXVec3Normalize(&vDir,&vDir);
	float fCos = D3DXVec3Dot(&vNormals[2],&vDir);
	if(fCos < 0.0f) 
		fCos = 0.0f;
	D3DXVECTOR3 vDist = g_stuLight.vPos - pRay->vIntersection;
	float dist = D3DXVec3Length(&vDist);
	float fIntensity = fCos*g_stuLight.fPower/dist;
	return fIntensity;
}
/*

*/
BOOL CRayRender::CheckRayIntersection(Ray* ray)
{
	return g_ClsTree.CheckRayIntersection(ray);
}

BOOL CRayRender::CheckShadow(D3DXVECTOR3* vPos)
{
	BOOL bIntersect = FALSE;
	D3DXVECTOR3 vDir = g_stuLight.vPos - *vPos;
	float fDist = D3DXVec3Length(&vDir);
	D3DXVec3Normalize(&vDir,&vDir);
	Ray stuShadowRay;
	stuShadowRay.vDir = vDir;
	stuShadowRay.vRayPos = *vPos;
	if(g_ClsTree.CheckRayIntersection(&stuShadowRay))
	{
		/*vDir = stuShadowRay.vRayPos - stuShadowRay.vIntersection;
		float dist = D3DXVec3Length(&vDir);
		if(fDist < dist)
			bIntersect = FALSE;
		else*/
			bIntersect = TRUE;
	}
	return bIntersect;
}

int CRayRender::GetIntersectionData(D3DXVECTOR3* vPos, D3DXVECTOR3* pNormal, int* face, Ray* ray)
{
	D3DXVECTOR3 vNormal = D3DXVECTOR3(0.0f,0.0f,0.0f);
	int faceId = ray->nFaceIndex;
	VERTEX* pVertices;
	int* pIndices;
	LockBuffersOfObject(&pVertices, &pIndices,ray->nObjectId);
	D3DXVECTOR3 vxes[3], vec3[2];
	for(int i=0; i<3; i++)
	{
		//vNormal += pVertices[pIndices[3*faceId+i]].Normal;
		vxes[i] = pVertices[pIndices[3*faceId+i]].Point;
	}
	UnlockBuffersOfObject(ray->nObjectId);
	vec3[0] = vxes[1] - vxes[0];
	vec3[1] = vxes[2] - vxes[0];
	D3DXVec3Cross(&vNormal,&vec3[0],&vec3[1]);
	D3DXVec3Normalize(&vNormal,&vNormal);
	*pNormal = vNormal;
	*face = faceId;
	*vPos = ray->vIntersection;
	g_arrObjects[ray->nObjectId]->pVertexBuffer->Unlock();
	g_arrObjects[ray->nObjectId]->pIndexBuffer->Unlock();
	return ray->nObjectId;
}

D3DXCOLOR CRayRender::RayTracing(Ray* ray)
{
	D3DXVECTOR3 vPos, vNormal;
	Ray stuShadowRay,stuReflectRay;
	D3DXCOLOR color=back_color;
	if(ray->nCount > MAX_COUNT)
	{
		return D3DXCOLOR(0.0f,0.0f,0.0f,1.0f);
	}
	ray->nCount++;
	if(g_ClsTree.CheckRayIntersection(ray))
	{
		ray->bLine = FALSE;
		//반사빛에 의한 색얻기
		D3DXCOLOR reflectColor;
		GetReflectedRay(&stuReflectRay,ray);
		stuReflectRay.nCount = ray->nCount;
		reflectColor = RayTracing(&stuReflectRay);
		float refI = 0.25f*(reflectColor.r+reflectColor.g+reflectColor.b)/3;
		int nObjectId = ray->nObjectId;
		int nMatId = g_arrObjects[nObjectId]->nMatIndex;
		reflectColor.r *= 0.25f*g_arrMats[nMatId].fSpecularColor[0]/255;
		reflectColor.g *= 0.25f*g_arrMats[nMatId].fSpecularColor[1]/255;
		reflectColor.b *= 0.25f*g_arrMats[nMatId].fSpecularColor[2]/255;
		int faceId;
		nObjectId = GetIntersectionData(&vPos,&vNormal,&faceId,ray);
		vPos += 0.01f*vNormal;
		if(CheckShadow(&vPos))//그림자검사
			color = D3DXCOLOR(0.2f,0.2f,0.2f,1.0f);
		else
		{
			//광원의 직접조명에 의한 색얻기
			float fIntensity = 0.0f;
			switch(m_nShadeType)
			{
			case 0:
				fIntensity = LambertIntensity(ray);
				break;
			case 1:
				fIntensity = GouraudIntensity(ray);
				break;
			default:
				fIntensity = PhongShading(ray);
				break;
			}
			fIntensity = fabs(fIntensity);
			int nMatIndex = g_arrObjects[nObjectId]->nMatIndex;
			color.r = fIntensity*g_arrMats[nMatIndex].fDiffuseColor[0];
			color.g = fIntensity*g_arrMats[nMatIndex].fDiffuseColor[1];
			color.b = fIntensity*g_arrMats[nMatIndex].fDiffuseColor[2];
			if(g_arrMats[nMatIndex].nType==XML_MT_IMAGE)
			{
				color = GetTextureColor(ray);
			}
		}
		color += reflectColor;
		color += D3DXCOLOR(refI,refI,refI,1.0f);
		if(color.r > 1.0f) color.r = 1.0f;
		if(color.g > 1.0f) color.g = 1.0f;
		if(color.b > 1.0f) color.b = 1.0f;
	}
	else
		color = back_color;
	return color;
}

BOOL CRayRender::LockBuffersOfObject(VERTEX** ppVertices, int** ppIndices, int objId)
{
	if(g_arrObjects[objId]->pVertexBuffer->Lock(0,0,(void**)ppVertices,D3DLOCK_DISCARD)!=D3D_OK)
		return FALSE;
	if(g_arrObjects[objId]->pIndexBuffer->Lock(0,0,(void**)ppIndices,D3DLOCK_DISCARD)!=D3D_OK)
		return FALSE;
	return TRUE;
}

void CRayRender::UnlockBuffersOfObject(int objId)
{
	g_arrObjects[objId]->pVertexBuffer->Unlock();
	g_arrObjects[objId]->pIndexBuffer->Unlock();
}

float CRayRender::LambertIntensity(Ray* ray)
{
	D3DXVECTOR3 vDist = g_stuLight.vPos - ray->vIntersection;
	float fDist = D3DXVec3Length(&vDist);
	D3DXVECTOR3 vNormal = D3DXVECTOR3(0.0f,0.0f,0.0f);
	int faceId = ray->nFaceIndex;
	VERTEX* pVertices;
	int* pIndices;
	LockBuffersOfObject(&pVertices, &pIndices, ray->nObjectId);
	D3DXVECTOR3 vxes[3];
	for(int i=0; i<3; i++)
	{
		//vNormal += pVertices[pIndices[3*faceId+i]].Normal;
		vxes[i] = pVertices[pIndices[3*faceId+i]].Point;
	}
	D3DXVECTOR3 vec3[2];
	vec3[0] = vxes[1] - vxes[0];
	vec3[1] = vxes[2] - vxes[0];
	D3DXVec3Cross(&vNormal,&vec3[0],&vec3[1]);
	D3DXVec3Normalize(&vNormal,&vNormal);
	UnlockBuffersOfObject(ray->nObjectId);
	D3DXVec3Normalize(&vDist,&vDist);
	float dot = D3DXVec3Dot(&vNormal,&vDist);
	if(dot<0.0f) 
		dot = 0.0f;
	float fIntensity = dot*g_stuLight.fPower/fDist;
	return fIntensity;
}

float CRayRender::GouraudIntensity(Ray* ray)
{
	float I[4],cx[3],rad[3];
	D3DXVECTOR3 vxs[5];
	/*정점들에서 세기계산,정점자리표들을 카메라자리표계에로 변환*/ 
	D3DXMATRIX mat;
	//D3DXMatrixLookAtLH(&mat, &g_stuCamera.vEyePos, &g_stuCamera.vLookAt, &g_stuCamera.vUp);
	D3DXVECTOR3 vLookAt = ray->vRayPos + ray->vDir;
	D3DXVECTOR3 vLeft,vUp;
	D3DXVec3Cross(&vLeft,&ray->vDir,&g_MyCameras[3].vUp);
	D3DXVec3Normalize(&vLeft,&vLeft);
	D3DXVec3Cross(&vUp,&vLeft,&ray->vDir);
	D3DXVec3Normalize(&vUp,&vUp);
	D3DXMatrixLookAtLH(&mat, &ray->vRayPos, &vLookAt, &vUp);
	D3DXVECTOR4 out;
	D3DXVECTOR3 vDir, vNormal, vert3D;
	int faceId = ray->nFaceIndex;
	float fDist;
	VERTEX* pVertices;
	int*    pIndices;
	LockBuffersOfObject(&pVertices, &pIndices, ray->nObjectId);
	for(int i=0; i<3; i++)
	{
		vert3D = pVertices[pIndices[3*faceId+i]].Point;
		vDir = g_stuLight.vPos - vert3D;
		fDist = D3DXVec3Length(&vDir);
		D3DXVec3Normalize(&vDir,&vDir);
		vNormal = -pVertices[pIndices[3*faceId+i]].Normal;
		D3DXVec3Normalize(&vNormal,&vNormal);
		I[i] = D3DXVec3Dot(&vDir,&vNormal);
		I[i] = g_stuLight.fPower*I[i]/fDist;
		if(I[i] < 0.0f) I[i] = 0.0f;
		D3DXVec3Transform(&out,&vert3D,&mat);
		vxs[i] = (D3DXVECTOR3)out;
	}
	UnlockBuffersOfObject(ray->nObjectId);
	I[3] = I[0];
	vxs[3] = vxs[0];
	D3DXVec3Transform(&out,&ray->vIntersection,&mat);
	vxs[4] = (D3DXVECTOR3)out;
	/*변환완료*/
	/*그로우방법으로 세기계산*/
	int k=0;
	for(int i=0; i<3; i++)
	{
		if(k==2) break;
		if((vxs[i].y < vxs[4].y && vxs[4].y < vxs[i+1].y) || (vxs[i].y > vxs[4].y && vxs[4].y > vxs[i+1].y))
		{
			float x[3],y[3],dx,dy[3];
			x[0] = vxs[i].x;	y[0] = vxs[i].y;	
			x[1] = vxs[i+1].x;	y[1] = vxs[i+1].y;
			x[2] = vxs[4].x;	y[2] = vxs[4].y;
			dx = x[0] - x[1];
			dy[0] = y[0] - y[1];
			dy[1] = y[0] - y[2];
			dy[2] = y[2] - y[1];
			//rad[k] = fabs((y[0]-y[2])/(y[0]-y[1]))*I[i+1] + fabs((y[2]-y[1])/(y[0]-y[1]))*I[i];
			//x[2] = (x[0]-x[1])*((y[2]-y[1])/(y[0]-y[1])+x[1]/(x[0]-x[1]));
			rad[k] = fabs(dy[1]/dy[0])*I[i+1] + fabs(dy[2]/dy[0])*I[i];
			x[2] = dx*dy[2]/dy[0] + x[1];
			cx[k] = x[2];
			k++;
		}
	}
	cx[2] = vxs[4].x;
	float l[3];
	l[0] = fabs(cx[2] - cx[0]);
	l[1] = fabs(cx[1] - cx[2]);
	l[2] = fabs(cx[1] - cx[0]);
	rad[2] = rad[0]*l[1]/l[2] + rad[1]*l[0]/l[2];
	return rad[2];
}

float CRayRender::PhongIntensity(Ray* ray)
{
	return 0.0f;
}

void CRayRender::SetShadeType(int nType)
{
	m_nShadeType = nType;
}

D3DXCOLOR CRayRender::GetTextureColor(Ray* ray)
{
	D3DXCOLOR color = D3DXCOLOR(0.0f,0.0f,0.0f,0.0f);
	int nMatIndex = g_arrObjects[ray->nObjectId]->nMatIndex;
	if(g_arrMats[nMatIndex].nType==XML_MT_IMAGE)
	{
		if(!g_arrMats[nMatIndex].pColors)
			return color;
		D3DXVECTOR2 txes[4];
		for(int i=0; i<3; i++)
		{
			txes[i] = ray->txes[i];
		}
		txes[3] = txes[0] + ray->u*(txes[1] - txes[0]) + ray->v*(txes[2] - txes[0]);
		txes[3].x = fabs(txes[3].x);
		txes[3].y = fabs(txes[3].y);
		int width = g_arrMats[nMatIndex].width;
		int height = g_arrMats[nMatIndex].height;
		int i = txes[3].x*width;
		int j = txes[3].y*height;
		if(i>g_arrMats[nMatIndex].width-1) 
			i = g_arrMats[nMatIndex].width-1;
		if(j>g_arrMats[nMatIndex].height-1) 
			j = g_arrMats[nMatIndex].height-1;
		int l = j*width + i;
		color = g_arrMats[nMatIndex].pColors[l];; 
	}
	return color;
}
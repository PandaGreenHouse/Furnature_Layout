#pragma once
#include "Model.h"
#include "MyArray.h"
#include "PlankModel.h"
class CModelObject
{
public:
	CModelObject(void);
	void CreateModelObject(LPDIRECT3DDEVICE9 pd3dDevice);
private:
	bool CreateMaterials(LPDIRECT3DDEVICE9 pd3dDevice);
	void TransformObject(ObjectNode* pNode);//D3DXMATRIX* matWorld, D3DXMATRIX* rotMat, IDirect3DVertexBuffer9* pVertexBuf );
	void ComputingNormal(ObjectNode* pNode);
	void ComputingRelativeCenter();
	//attributes
private:
	int			m_intRootCount;
	CMyArray<Model_Node> m_ArrRootNodes;
	CMyArray<ModelInfo> m_ArrModelInfoes;
	LPDIRECT3DTEXTURE9* m_ppTextures;
	CPlankModel			m_ClsPlank;
};
extern CModelObject g_ClsModelObject;


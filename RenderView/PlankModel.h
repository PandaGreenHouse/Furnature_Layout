#pragma once
#include "ObjectNode.h"
class CPlankModel
{
public:
	CPlankModel(void);
	void CreatePlankModel(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode);
	void ComputingBoundingBox(ObjectNode* pNode);
	void ComputingOutlineModel(ObjectNode* pNode);
private:
	void CalculateVertices(float width, float depth, float height);
private:
	D3DXVECTOR3 m_d3dVertices[8];
};


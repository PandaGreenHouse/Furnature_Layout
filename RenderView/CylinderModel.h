#pragma once
#include "ObjectNode.h"
class CCylinderModel
{
public:
	CCylinderModel(void);
	void CreateModel(LPDIRECT3DDEVICE9 pd3dDevice, ObjectNode* pNode, int nSlices);
	void ComputingTextureCoordinates(ObjectNode* pNode);
};


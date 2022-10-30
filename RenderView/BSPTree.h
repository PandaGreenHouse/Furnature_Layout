#pragma once

class CBSPTree
{
public:
	CBSPTree(void);
	~CBSPTree(void);
	void CreateTree();
	void FreeTree();
	BOOL CheckRayIntersection(Ray* pRay);
//Attributes
protected:
	BSPTreeNode* m_pRoot;
	float	  m_fLimSize;	
	int		  m_intNodeCount;
	int		  m_intLeafCount;
//Implementation
protected:
	BOOL GetVerticesOfTri(int intObject, int face, VERTEX* vertices);
	void GetBoundingBox(D3DXVECTOR3* vMin, D3DXVECTOR3* vMax);
	BSPTreeNode* LocateLeaf(BSPTreeNode* pNode, D3DXVECTOR3* vert3D);
	D3DXVECTOR3 GetEntry(BSPTreeNode* pNode, D3DXVECTOR3* vPos, D3DXVECTOR3* vDir);
	D3DXVECTOR3 GetExit(BSPTreeNode* pNode, D3DXVECTOR3* vPos, D3DXVECTOR3* vDir);
	BOOL IsInAABB(D3DXVECTOR3* vert3D, D3DXVECTOR3* vMin, D3DXVECTOR3* vMax);
	BOOL ProbeNode(BSPTreeNode* pNode, Ray* pRay);
	void FreeNode(BSPTreeNode* pNode);
	float GetLimitSize(BSPTreeNode* pNode);
	void CreateRootNode();
	void CreateChildren(BSPTreeNode* pNode);
	void GetVerticesOfAABBOfNode(BSPTreeNode* pNode);
	void PartitionXY(FaceInfo* pFaces2, int* nCount2,
					 FaceInfo* pFaces1, int* nCount1,
					 FaceInfo* pFaces0,	int nCount0,float cz);
	void PartitionYZ(FaceInfo* pFaces2, int* nCount2,
					 FaceInfo* pFaces1,	int* nCount1,
					 FaceInfo* pFaces0,	int nCount0,float cx);
	void PartitionZX(FaceInfo* pFaces2, int* nCount2,
					 FaceInfo* pFaces1,	int* nCount1,
					 FaceInfo* pFaces0,	int nCount0,float cy);
	void TraverseNode(Ray* pRay);
	BOOL CheckIntersectLeaf(BSPTreeNode* pNode, Ray* pRay);
	void CheckNode(BSPTreeNode* pNode, Ray* pRay);
};


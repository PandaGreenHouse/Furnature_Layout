#pragma once
#ifndef __MYARRAY_CLASS_HEADER__
#define	__MYARRAY_CLASS_HEADER__

#include <stdlib.h>

template <class T>
class CMyArray
{
public:
	CMyArray(void);
	~CMyArray(void);

	CMyArray(int nInitSize, int nGrowSize);

	BOOL Create(int nInitSize, int nGrowSize);
	void Release();

	BOOL AddItem(T* pItems, int nCount = 1);
	BOOL AddAt(int nIndex, T* pItems, int nCount = 1);
	BOOL Delete(int nStart, int nCount = 1);
	void Clear();
	T*	 Copy(int nStart = 0, int nCount = -1);

	T*	 GetItem(int index = 0);
	T*	 GetLast();
	int  GetCount();
	BOOL IsExist(T* pValue);
	int  GetIndex(T* pValue, int nStart = 0, BOOL bAddNew = TRUE);

	const T& operator[](INT_PTR nIndex) const;
	T& operator[](INT_PTR nIndex);

private:
	BOOL AllocMem(int nNewSize);
	void FreeMem();

	int	m_nMaxSize;		//Max Item Count of the Array
	int	m_nUsedSize;	//Used Item of the Array
	int	m_nGrowSize;	//Array Grow Size, the newly allocated unit count
	T*	m_pData;		//Array Data Pointer
};
extern CMyArray<D3DXVECTOR3> g_ArrayVert3D;
extern CMyArray<CMyArray<D3DXVECTOR3>> g_ArrayLines;
template <class T>
CMyArray<T>::CMyArray(void)
{
	m_pData	= NULL;
	m_nMaxSize	= 0;
	m_nUsedSize = 0;
	m_nGrowSize	= 16;
}

template <class T>
CMyArray<T>::CMyArray(int nInitSize, int nGrowSize)
{
	m_pData		= NULL;
	m_nMaxSize	= 0;
	Create(nInitSize, nGrowSize);
}

template <class T>
CMyArray<T>::~CMyArray(void)
{
	FreeMem();
}

template <class T>
BOOL CMyArray<T>::Create(int nInitSize, int nGrowSize)
{
	m_nGrowSize	= nGrowSize;
	return AllocMem(nInitSize);
}

template <class T>
void CMyArray<T>::Release()
{
	FreeMem();
}

template <class T>
BOOL CMyArray<T>::AddItem(T* pItems, int nCount)
{
	if (pItems == NULL || nCount < 1)
		return FALSE;

	//Allocate for New Item Memory
	if (m_pData == NULL)
	{
		m_nMaxSize	= 0;
		m_nUsedSize = 0;
	}
	if (!AllocMem(nCount))
		return FALSE;
	//Copy Data to New Buffer
	memcpy(m_pData + m_nUsedSize, pItems, nCount * sizeof(T));
	//Increase Used Item Count
	m_nUsedSize += nCount;

	return TRUE;
}

template <class T>
BOOL CMyArray<T>::AddAt(int nIndex, T* pItems, int nCount)
{
	T	*pSrc, *pDst, *pEnd;
	if (pItems == NULL || nCount < 1 || nIndex < 0)
		return FALSE;

	//Allocate for New Item Memory
	if (m_pData == NULL)
	{
		m_nMaxSize	= 0;
		m_nUsedSize = 0;
	}
	if (!AllocMem(nCount))
		return FALSE;
	//Shift elements after nIndex
	pSrc = m_pData + m_nUsedSize - 1;
	pDst = pSrc + nCount;
	pEnd = m_pData + nIndex;
	while (pSrc >= pEnd)
	{
		*pDst = *pSrc;
		pSrc--; pDst--;
	}
	//Copy Data at nIndex
	memcpy(m_pData + nIndex, pItems, nCount * sizeof(T));
	//Increase Used Item Count
	m_nUsedSize += nCount;

	return TRUE;
}

template <class T>
BOOL CMyArray<T>::Delete(int nStart, int nCount)
{
	T	*pSrc, *pDst, *pEnd;
	int	nItems;

	if (m_pData == NULL || nStart < 0 || nCount < 1)
		return FALSE;

	//Get Delete Item Count
	nItems	= m_nUsedSize - nStart;
	if (nItems > nCount)
		nItems = nCount;

	//Copy Items after delete items
	pDst = m_pData + nStart;
	pSrc = m_pData + nStart + nItems;
	pEnd = m_pData + m_nUsedSize;
	for (; pSrc < pEnd; pSrc++, pDst++)
		*pDst = *pSrc;

	//Subtract Deleted Item Count from Used Size
	m_nUsedSize -= nItems;
	return TRUE;
}

template <class T>
void CMyArray<T>::Clear()
{
	if (m_pData && (m_nUsedSize > 0))
		memset(m_pData, 0, m_nUsedSize * sizeof(T));
	m_nUsedSize	= 0;
}

template <class T>
T*	 CMyArray<T>::Copy(int nStart, int nCount)
{
	T*	pData = NULL;

	if (m_pData == NULL || m_nUsedSize < 1)
		return NULL;
	if (nStart < 0)
		nStart = 0;
	if (nCount < 0)
		nCount = m_nUsedSize;

	pData	= (T*) calloc(m_nUsedSize, sizeof(T));
	if (pData)
		memcpy(pData, m_pData, m_nUsedSize * sizeof(T));
	return pData;
}

template <class T>
T* CMyArray<T>::GetItem(int index)
{
	return (m_pData + index);
}

template <class T>
T&	CMyArray<T>::operator[](int index)
{
	return (m_pData[index]);
}

template <class T>
T* CMyArray<T>::GetLast()
{
	return (m_pData + m_nUsedSize - 1);
}

template <class T>
int  CMyArray<T>::GetCount()
{
	return m_nUsedSize;
}

template <class T>
BOOL CMyArray<T>::IsExist(T* pValue)
{
	T*	 pCur = m_pData;

	if (m_pData == NULL || pValue == NULL || m_nUsedSize < 1)
		return FALSE;

	for (int k = 0; k < m_nUsedSize; k++, pCur++)
	{
		if (memcmp(pCur, pValue, sizeof(T)) == 0)
			return TRUE;
	}

	return FALSE;
}

template <class T>
int	 CMyArray<T>::GetIndex(T* pValue, int nStart, BOOL bAddNew)
{
	T*	pCur = NULL;
	int	k;

	if (m_pData == NULL || pValue == NULL || nStart < 0)
		return -1;

	pCur = m_pData + nStart;
	for (k = nStart; k < m_nUsedSize; k++, pCur++)
	{
		if (memcmp(pCur, pValue, sizeof(T)) == 0)
			break;
	}

	if (k >= m_nUsedSize)
	{
		if (bAddNew)
			bAddNew = AddItem(pValue, 1);
		if (bAddNew)
			k = m_nUsedSize - 1;
		else
			k = -1;
	}

	return k;
}


template <class T>
BOOL CMyArray<T>::AllocMem(int nNewSize)
{
	T*	pNewData;
	int	nTotal;

	nTotal = nNewSize + m_nUsedSize;
	if ((nTotal > m_nMaxSize) || (m_pData == NULL))
	{
		//Get Total Size considering Grow Size
		if (m_nGrowSize > 1)
			nTotal = (int) ((nTotal + m_nGrowSize - 1) / m_nGrowSize) * m_nGrowSize;
		//Allocate New Buffer
		pNewData = (T*) calloc(nTotal, sizeof(T));
		if (pNewData == NULL)
			return FALSE;
		//Copy Used Item Data From Old Buffer and Free Old Buffer
		if (m_pData)
		{
			if (m_nUsedSize > 0)
				memcpy(pNewData, m_pData, m_nUsedSize * sizeof(T));
			for( int i = 0; i < m_nUsedSize; i++ )
				(m_pData + i)->~T();
			free(m_pData);
		}
		//Set New Buffer Address and Max Item Count
		m_pData		= pNewData;
		m_nMaxSize	= nTotal;
	}

	return (m_pData != NULL);
}

template <class T>
void CMyArray<T>::FreeMem()
{
	if (m_pData)
	{
		free(m_pData);
		m_pData = NULL;
	}

	m_nMaxSize	= 0;
	m_nUsedSize	= 0;
	m_nGrowSize	= 0;
}

#endif //__MYARRAY_CLASS_HEADER__
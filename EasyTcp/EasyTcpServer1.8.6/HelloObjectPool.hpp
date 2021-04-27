#ifndef _HELLOOBJECTPOOL_HPP_
#define _HELLOOBJECTPOOL_HPP_

#include <stdlib.h>
#include <assert.h>
#include <mutex>

#ifdef _DEBUG
#ifndef xPrintf
#include<stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#endif
#else
#ifndef xPrintf
#define xPrintf(...)
#endif
#endif // _DEBUG

template<class Type, size_t nPoolSize>
class CELLObjectPool
{
public:
	CELLObjectPool()
	{
		_pBuf = nullptr;
		initPool();
	}
	~CELLObjectPool()
	{
		if (_pBuf)
			delete[] _pBuf;
	}
private:
	class NodeHeader
	{
	public:
		// 下一块位置
		NodeHeader *pNext;
		// 内存块编号
		int nID;
		// 引用次数
		char nRef;
		// 是否在内存池
		bool bPool;
	private:
		char c1;
		char c2;
	};
public:
	//釋放對象
	void freeObjMemory(void* pMem)
	{
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		xPrintf("freeObjMemory: %llx, id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);

		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_mutex);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			delete[] pBlock;
		}
	}

	// 申请對象内存
	void* allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(_mutex);
		NodeHeader* pReturn = nullptr;
		// 申请不符合内存单元规格 或者内存池已满
		if (nullptr == _pHeader)
		{
			pReturn = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		//xPrintf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
private:
	//初始化對象
	void initPool()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		// 計算對象池大小
		size_t realSize = sizeof(Type) + sizeof(NodeHeader);
		size_t n = nPoolSize * realSize;
		// 有先向内存池申請内存
		_pBuf = new char[n];

		_pHeader = (NodeHeader*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pNext = nullptr;

		//初始化每个块的信息
		NodeHeader* pTemp1 = _pHeader;
		for (size_t n = 1; n < nPoolSize; n++)
		{
			NodeHeader* pTemp2 = (NodeHeader*)(_pBuf + (n * realSize));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

private:
	NodeHeader* _pHeader;
	// 對象池地址
	char* _pBuf;
	std::mutex _mutex;
};

template<class Type, size_t nPoolSize>
class ObjectPoolBase
{
public:
	void* operator new(size_t nSize)
	{
		return ObjectPool().allocObjMemory(nSize);
	}

	void operator delete(void *p)
	{
		ObjectPool().freeObjMemory(p);
	}
	// 可變參函數模板
	template<typename ...Args>
	static Type* createObject(Args ... args)
	{
		Type* obj = new Type(args...);
		return obj;
	}
	static void destroyObject(Type* obj)
	{
		delete obj;
	}
private:
	typedef CELLObjectPool<Type, nPoolSize> ClassTypePool;
	static ClassTypePool& ObjectPool()
	{
		// 單例對象池對象
		static ClassTypePool sPool;
		return sPool;
	}
};

#endif
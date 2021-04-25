#ifndef _MEMORYMGR_H_
#define _MEMORYMGT_H_
#include "Alloctor.h"
#include <assert.h> //调试使用

#ifdef _DEBUG
#include <stdio.h>
#define xPrintf(...) printf(__VA_ARGS__)
#else
#define xPrintf(...)
#endif

#define  MAX_MEMORY_SIZE 1024

class MemoryAlloc; // 提前声明

class MemoryBlock
{
public:
	// 所属内存池
	MemoryAlloc* pAlloc;
	// 下一块位置
	MemoryBlock *pNext;
	// 是否在内存池
	bool bPool;
	// 内存块编号
	int nID;
	// 引用次数
	int nRef;
private:
	// 预留 内存补齐 32bit 4字节对齐 64bit 8字节对齐
	char cNULL1;
	char cNULL2;
	char cNULL3;
};

class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlockSize = 0;
	};
	~MemoryAlloc()
	{
		if (_pBuf)
			free(_pBuf);
	};
	// 申请内存
	void* allocMemory(size_t nSize)
	{
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		// 申请不符合内存单元规格 或者内存池已满
		if (!_pHeader)
		{
			pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)
			return;

		if (pBlock->bPool)
		{
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
			free(pBlock);
	}

	void initMemory()
	{
		assert(nullptr == _pBuf);
		if (_pBuf)
			return;
		// 1 计算池大小,向系统申请内存
		size_t realSize = _nSize + sizeof(MemoryBlock);
		size_t bufSize = (_nSize + sizeof(MemoryBlock))* _nBlockSize;
		_pBuf = (char*)malloc(bufSize);
		// 2 初始化第一个的信息
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;

		//初始化每个块的信息
		MemoryBlock* pTemp1 = _pHeader;
		for (size_t n = 1; n < _nBlockSize; n++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n*realSize));
			pTemp2->bPool = true;
			pTemp2->nID = n;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}

protected:
	// 内存池地址
	char* _pBuf;
	// 头部内存单元
	MemoryBlock* _pHeader;
	// 内存单元大小
	size_t _nSize;
	// 内存单元数量
	size_t _nBlockSize;
};

template<size_t nSize, size_t nBlockSize>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		// 动态 补齐,内存对齐
		const size_t n = sizeof(void*); // 动态的, 32bit sizeof(void*) = 4,64bit sizeof(void*) = 8

		_nSize = (nSize / n)*n + (nSize%n ? n : 0);
		_nBlockSize = nBlockSize;
	}
};

// 管理工具
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init_szAlloc(0, 64, &_mem64);
		init_szAlloc(65, 128, &_mem128);
		init_szAlloc(129, 256, &_mem256);
		init_szAlloc(257, 512, &_mem512);
		init_szAlloc(513, 1024, &_mem1024);
	};
	~MemoryMgr()
	{
	};
public:
	// 单例模式 保证全局唯一
	static MemoryMgr& Instance()
	{
		static MemoryMgr mgr;
		return mgr;
	}
	// 申请内存
	void* allocMem(size_t nSize)
	{
		// 还有/符合条件
		if (nSize <= MAX_MEMORY_SIZE)
		{
			_szAlloc[nSize]->allocMemory(nSize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr; // 仍然接受内存管理工具管理
			pReturn->pNext = nullptr;
			xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}
	}

	void freeMem(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);

		if (pBlock->bPool)
			pBlock->pAlloc->freeMemory(pMem);
		else
		{
			// 引用次数为0 释放
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}
	// 共享内存 添加引用
	void addRef(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		pBlock->nRef++;
	}
private:
	// 初始化内存池映射表
	void init_szAlloc(int nBegin, int nEnd, MemoryAlloc* pMemA)
	{
		for (int n = nBegin; n <= nEnd; n++)
		{
			_szAlloc[n] = pMemA;
		}
	}
private:
	// 内存池建立
	MemoryAlloctor<64, 1000000> _mem64;
	MemoryAlloctor<128, 1000000> _mem128;
	MemoryAlloctor<256, 1000000> _mem256;
	MemoryAlloctor<512, 1000000> _mem512;
	MemoryAlloctor<1024, 1000000> _mem1024;

	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};

#endif
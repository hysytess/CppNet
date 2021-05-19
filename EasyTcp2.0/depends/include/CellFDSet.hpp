#ifndef _CELL_FDSET_HPP_
#define _CELL_FDSET_HPP_

#include "PublicLib.hpp"

#define  MAX_FD 10240

// 自定义 fd_set
class CellFDSet
{
public:
	CellFDSet()
	{
		int nSocketNum = MAX_FD;
#ifdef _WIN32
		_nfdsize = sizeof(u_int) + sizeof(SOCKET) * nSocketNum;
#else
		_nfdsize = nSocketNum / (8*sizeof(char));
#endif
		_pfdset = (fd_set*)new char[_nfdsize];
		memset(_pfdset, 0, _nfdsize);
	}
	~CellFDSet() 
	{
		if (_pfdset)
		{
			delete[] _pfdset;
			_pfdset = nullptr;
		}
	}

	inline void add(SOCKET s)
	{
#ifdef _WIN32 
	FD_SET(s, _pfdset);
#else
		if ((int)s < MAX_FD)
		{
			FD_SET(s, _pfdset);
		}
		else
		{
			CellLog_Error("CellFDSet::add socket<%d>, MAX_FD<%d>", (int)s, MAX_FD);
		}
#endif
	}

	inline void del(SOCKET s)
	{
		FD_CLR(s, _pfdset);
	}

	inline void zero()
	{
#ifdef _WIN32
		FD_ZERO(_pfdset);
#else
		//linux 中的FD_ZERO 在此处使用不安全, 因为集合大小已经超过1024
		memset(_pfdset, 0, _nfdsize);
#endif
	}

	inline bool has(SOCKET s)
	{
		return FD_ISSET(s,_pfdset);
	}

	inline fd_set* fdset()
	{
		return _pfdset;
	}

	void copy(CellFDSet& set)
	{
		memcpy(_pfdset, set.fdset(), set._nfdsize);
	}

private:
	fd_set* _pfdset = nullptr;
	size_t _nfdsize = 0;
};

#endif

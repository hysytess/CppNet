#ifndef _CELL_FDSET_HPP_
#define _CELL_FDSET_HPP_

#include"CELL.hpp"

#define CELL_MAX_FD 10240

class CELLFDSet
{
public:
	CELLFDSet()
	{

	}

	~CELLFDSet()
	{
		destory();
	}

	void create(int MaxFds)
	{
		int nSocketNum = MaxFds;
#ifdef _WIN32
		if (nSocketNum < 64)
			nSocketNum = 64;
		_nfdSize = sizeof(u_int) + (sizeof(SOCKET)*nSocketNum);
#else
		// 8kb存储65535 个 sockfd 8192byte*8 = 65536
		if (nSocketNum < 65535)
			nSocketNum = 65535;
		_nfdSize = nSocketNum / (8 * sizeof(char))+1;
#endif // _WIN32
		_pfdset = (fd_set *)new char[_nfdSize];
		memset(_pfdset, 0, _nfdSize);
		_MAX_SOCK_FD = nSocketNum;
	}

	void destory()
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
		if(s < _MAX_SOCK_FD)
		{
			FD_SET(s, _pfdset);
		}else{
			CELLLog_Error("CELLFDSet::add sock<%d>, CELL_MAX_FD<%d>",(int)s,CELL_MAX_FD);
		}
#endif // _WIN32
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
		memset(_pfdset, 0, _nfdSize);
#endif // _WIN32
	}

	inline bool has(SOCKET s)
	{
		return FD_ISSET(s, _pfdset);
	}

	inline fd_set* fdset()
	{
		return _pfdset;
	}

	void copy(CELLFDSet& set)
	{
		memcpy(_pfdset, set.fdset(), set._nfdSize);
	}
private:
	fd_set * _pfdset = nullptr;
	size_t _nfdSize = 0;
	int _MAX_SOCK_FD = 0;
};


#endif // !_CELL_FDSET_HPP_

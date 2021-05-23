#ifndef _EasyTcpClient_Select_hpp_
#define _EasyTcpClient_Select_hpp_

#include "EasyTcpClient2_4.hpp"

class EasyTcpClient_Select:public EasyTcpClient
{
public:

	bool OnRun(int microseconds = 1)
	{
		if (isRun())
		{
			SOCKET sock = _pClient->sockfd();
			
			_fdReads.zero();
			_fdReads.add(sock);

			_fdWrite.zero();

			timeval tv = { 0,microseconds };
			int ret = 0;
			if (_pClient->needWrite())
			{
				_fdWrite.add(sock);
				ret = (int)select(sock + 1, _fdReads.fdset(), _fdWrite.fdset(), 0, &tv);
			}
			else
			{
				ret = (int)select(sock + 1, _fdReads.fdset(), nullptr, nullptr, &tv);
			}
			
			if (ret < 0)
			{
				CellLog_Debug("<socket=%d>select done. terminal code:1", (int)sock);
				Close();
				return false;
			}
			if (_fdReads.has(sock))
			{
				//FD_CLR(_sock, &fdReads);
				if (SOCKET_ERROR == RecvData())
				{
					CellLog_Debug("<socket=%d>select done. terminal code:2", (int)sock);
					Close();
					return false;
				}
			}

			if (_fdWrite.has(sock))
			{
				//FD_CLR(_sock, &fdReads);
				if (SOCKET_ERROR == _pClient->SendDataReal()) // 
				{
					Close();
					return false;
				}
			}

			return true;
		}
		return false;
	}


private:

	CellFDSet _fdReads;
	CellFDSet _fdWrite;
};

#endif
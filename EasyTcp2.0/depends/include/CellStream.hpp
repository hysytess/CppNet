#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_

//Byte Stream
class CellStream
{
public:
//	用户自定义的流缓冲区:外部缓冲区地址  外部缓冲区大小  是否需要自动释放
	CellStream(char* pData, int nSize = 1024, bool bDelete = false)
	{
		_nSize = nSize;
		_pBuff = pData;
		_bDelete = bDelete;
	}

	CellStream(int nSize = 1024)
	{ 
		_nSize = nSize;
		_pBuff = new char[_nSize];
		_bDelete = true;
	}
	~CellStream()
	{ 
		if (_bDelete && _pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}
public:
//  Read byte stream
	template<typename T>
	bool Read(T& n)
	{
		auto nLen = sizeof(T);
		// 判断能否读,是否在缓冲区区中[位置是否正确]
		if (_nReadPos + nLen <= _nSize)
		{
			// 将要读取的数据拷贝出来
			memcpy(&n, _pBuff + _nReadPos, nLen);
			_nReadPos += nLen;
			return true;
		}
		return false;
	}

	int8_t ReadInt8(int8_t def = 0)
	{
		Read(def);
		return def;
	}

	int16_t ReadInt16(int16_t def = 0)
	{
		Read(def);
		return def;
	}

	int32_t ReadInt32(int32_t def = 0)
	{
		Read(def);
		return def;
	}

	int64_t ReadInt64(int64_t def = 0)
	{
		Read(def);
		return def;
	}

	float ReadFloat(float def = 0.0f)
	{
		Read(def);
		return def;
	}

	double ReadDouble(double def = 0.0)
	{
		Read(def);
		return def;
	}


//  Write byte stream
	template<typename T>
	bool Write(T n)
	{
		//计算写入字节长度
		auto nLen = sizeof(T);
		//判读能否写入
		if (_nWritePos + nLen <= _nSize)
		{
			memcpy(_pBuff + _nWritePos, &n, nLen);
			_nWritePos += nLen;
			return true;
		}
		return false;
	}
	template<typename T>
	bool WriteArray(T* array, uint32_t len)
	{
		//计算写入数组字节长度
		auto nLen = sizeof(T)*len;
		//判读能否写入
		if (_nWritePos + nLen + sizeof(uint32_t)<= _nSize)
		{
			WriteInt32(len);
			memcpy(_pBuff + _nWritePos, array, nLen);
			_nWritePos += nLen;
			return true;
		}
		return false;
	}

	// char
	bool WriteInt8(int8_t n)
	{
		return Write<int8_t>(n); //外部使用 严格限定内容时使用
		//return Write(n);
	}
	// short
	bool WriteInt16(int16_t n)
	{
		return Write<int16_t>(n);
		//return Write(n);
	}
	// int
	bool WriteInt32(int32_t n)
	{
		return Write<int32_t>(n);
		//return Write(n);
	}
	// long
	bool WriteInt64(int64_t n)
	{
		return Write<int64_t>(n);
		//return Write(n);
	}

	float WriteFloat(float n)
	{
		return Write<float>(n);
		//return Write(n);
	}
	double WriteDouble(double n)
	{
		return Write<double>(n);
		//return Write(n);
	}

private:
//  字节流缓冲区
	char* _pBuff = nullptr;
//  字节流缓冲区大小
	int _nSize = 0;
//  字节流缓冲区已写入数据的尾部位置,已写入长度
	int _nWritePos = 0;
//  已读取数据尾部
	int _nReadPos = 0;

	bool _bDelete = false;
};
#endif

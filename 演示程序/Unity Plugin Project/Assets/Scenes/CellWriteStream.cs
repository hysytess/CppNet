using System;
using UnityEngine;
using System.Text;
using System.Runtime.InteropServices;

public class CellWriteStream
{
    //ctrl + m 折叠
    // sbyte = int8_t
    // byte = uint8_t
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern IntPtr CellWriteStream_Create(int nSize);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteInt8(IntPtr cppwStremobj,sbyte n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteUInt8(IntPtr cppwStremobj, byte n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteInt16(IntPtr cppwStremobj, Int16 n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteUInt16(IntPtr cppwStremobj, UInt16 n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteInt32(IntPtr cppwStremobj, Int32 n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteUInt32(IntPtr cppwStremobj, UInt32 n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteInt64(IntPtr cppwStremobj, Int64 n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteUInt64(IntPtr cppwStremobj, UInt64 n);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteFloat(IntPtr cppwStremobj, float n);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteDouble(IntPtr cppwStremobj, double n);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellWriteStream_WriteString(IntPtr cppwStremobj, string str);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern void CellWriteStream_Release(IntPtr cppwStremobj);
    //-------------------------------------------------------------------------------------------------//

    private IntPtr _cppwStreamObj = IntPtr.Zero;
    public CellWriteStream(int nSize = 1024)
    {
        _cppwStreamObj = CellWriteStream_Create(nSize);
    }

    public IntPtr cppObj
    {
        get
        {
            return _cppwStreamObj;
        }
    }

    public void setNetCmd(NetCMD cmd)
    {
        WriteUInt16((UInt16)cmd);
    }

    public void finsh()
    {
    }

    public void WriteInt8(sbyte n)
    {
        CellWriteStream_WriteInt8(_cppwStreamObj, n);
    }
    public void WriteUInt8(byte n)
    {
        CellWriteStream_WriteUInt8(_cppwStreamObj, n);

    }

    public void WriteInt16(Int16 n)
    {
        CellWriteStream_WriteInt16(_cppwStreamObj, n);
    }
    public void WriteUInt16(UInt16 n)
    {
        CellWriteStream_WriteUInt16(_cppwStreamObj, n);
    }

    public void WriteInt32(Int32 n)
    {
        CellWriteStream_WriteInt32(_cppwStreamObj, n);
    }
    public void WriteUInt32(UInt32 n)
    {
        CellWriteStream_WriteUInt32(_cppwStreamObj, n);
    }

    public void WriteInt64(Int64 n)
    {
        CellWriteStream_WriteInt64(_cppwStreamObj, n);
    }
    public void WriteUInt64(UInt64 n)
    {
        CellWriteStream_WriteUInt64(_cppwStreamObj, n);

    }

    public void WriteFloat(float n)
    {
        CellWriteStream_WriteFloat(_cppwStreamObj, n);

    }
    public void WriteDouble(double n)
    {
        CellWriteStream_WriteDouble(_cppwStreamObj, n);
    }

    public void WriteString(string s)
    {
        byte[] buffer = Encoding.UTF8.GetBytes(s);
        WriteUInt32((UInt32)buffer.Length + 1);
        for(int n = 0;n<buffer.Length;n++)
        {
            WriteUInt8(buffer[n]);
        }
        WriteUInt8(0);
    }

    public void WriteBytes(byte[] data)
    {
        WriteUInt32((UInt32)data.Length + 1);
        for (int n = 0; n < data.Length; n++)
        {
            WriteUInt8(data[n]);
        }
    }

    public void WriteInt32Arr(Int32[] data)
    {
        WriteUInt32((UInt32)data.Length);
        for (int n = 0; n < data.Length; n++)
        {
            WriteInt32(data[n]);
        }
    }


    public void Release()
    {
        CellWriteStream_Release(_cppwStreamObj);
    }

}
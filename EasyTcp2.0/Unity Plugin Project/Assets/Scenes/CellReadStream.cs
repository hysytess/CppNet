using System;
using UnityEngine;
using System.Text;
using System.Runtime.InteropServices;

public class CellReadStream
{
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern IntPtr CellReadStream_Create(IntPtr str,int le);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern sbyte CellReadStream_ReadInt8(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern byte CellReadStream_ReadUInt8(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern Int16 CellReadStream_ReadInt16(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern UInt16 CellReadStream_ReadUInt16(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern Int32 CellReadStream_ReadInt32(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern UInt32 CellReadStream_ReadUInt32(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern Int64 CellReadStream_ReadInt64(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern UInt64 CellReadStream_ReadUInt64(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern float CellReadStream_ReadFloat(IntPtr cpprStremobj);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern double CellReadStream_ReadDouble(IntPtr cpprStremobj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern bool CellReadStream_ReadString(IntPtr cpprStremobj, StringBuilder buffer, int len);
#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern UInt32 CellReadStream_OnlyReadUint32(IntPtr cpprStremobj);

#if (UNITY_IPHONE || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport("__Internal")]
#else
    [DllImport("EasyTcpClientPulgin")]
#endif
    private static extern void CellReadStream_Release(IntPtr cpprStremobj);
    //--------------------------------------------------------------------------------------------------------------//


    private IntPtr _cpprStremobj = IntPtr.Zero;
    /// <summary>
    /// 
    /// </summary>
    /// <param name="data">C++ 消息回调传入的消息数据指针</param>
    /// <param name="len">数据字节长度</param>
    public CellReadStream(IntPtr data,int len)
    {
        _cpprStremobj = CellReadStream_Create(data, len);
    }
    public NetCMD ReadNetCmd()
    {
        return (NetCMD)ReadUInt16();
    }

    public sbyte ReadInt8(sbyte n = 0)
    {
        return CellReadStream_ReadInt8(_cpprStremobj);
    }
    public Int16 ReadInt16(Int16 n = 0)
    {
        return CellReadStream_ReadInt16(_cpprStremobj);
    }
    public Int32 ReadInt32(Int32 n = 0)
    {
        return CellReadStream_ReadInt32(_cpprStremobj);
    }
    public Int64 ReadInt64(Int64 n = 0)
    {
        return CellReadStream_ReadInt64(_cpprStremobj);
    }

    public byte ReadUInt8(byte n = 0)
    {
        return CellReadStream_ReadUInt8(_cpprStremobj);
    }
    public UInt16 ReadUInt16(UInt16 n = 0)
    {
        return CellReadStream_ReadUInt16(_cpprStremobj);
    }
    public UInt32 ReadUInt32(UInt32 n = 0)
    {
        return CellReadStream_ReadUInt32(_cpprStremobj);
    }
    public UInt64 ReadUInt64(UInt64 n = 0)
    {
        return CellReadStream_ReadUInt64(_cpprStremobj);
    }

    public float ReadFloat(float n = 0.0f)
    {
        return CellReadStream_ReadFloat(_cpprStremobj);
    }
    public double ReadDouble(double n = 0.0)
    {
        return CellReadStream_ReadDouble(_cpprStremobj);
    }

    public UInt32 OnlyReadUInt32(UInt32 n = 0)
    {
        return CellReadStream_OnlyReadUint32(_cpprStremobj);
    }

    public string ReadString()
    {
        //Int32 len = (Int32)OnlyReadUInt32();
        //StringBuilder sb = new StringBuilder(len);
        //CellReadStream_ReadString(_cpprStremobj, sb, len);
        //return sb.ToString();

        Int32 len = (Int32)ReadInt32();
        byte[] buffer = new byte[len];
        for(int n = 0;n < len;n++)
        {
            buffer[n] = ReadUInt8();
        }
        return Encoding.UTF8.GetString(buffer, 0, len);
    }
    public Int32[] ReadInt32Array()
    {
        int len = ReadInt32();
        Int32[] data = new Int32[len];
        for(int n = 0;n < len; n++)
        {
            data[n] = ReadInt32();
        }
        return data;
    }

    public void Release()
    {
        CellReadStream_Release(_cpprStremobj);
    }
}

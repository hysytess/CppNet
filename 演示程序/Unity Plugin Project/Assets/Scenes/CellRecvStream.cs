using System;
using UnityEngine;
using System.Text;
using System.Runtime.InteropServices;

public class CellRecvStream
{

    private byte[] _buffer = null;
    private int _nReadPos = 0;
    public CellRecvStream(IntPtr data, int len)
    {
        _buffer = new byte[len];
        Marshal.Copy(data, _buffer, 0, len);
       
    }
    private void Pop(int n)
    {
        _nReadPos += n;
    }
    private bool canRead(int n)
    {
        return _buffer.Length - _nReadPos >= n;
    }

    public sbyte ReadInt8(sbyte n = 0)
    {
        if (canRead(1))
        {
            n = (sbyte)_buffer[_nReadPos];
            Pop(1);
        }
        return n;
    }
    public Int16 ReadInt16(Int16 n = 0)
    {
        if (canRead(2))
        {
            n = BitConverter.ToInt16(_buffer, _nReadPos);
            Pop(2);
        }
        return n;
    }
    public Int32 ReadInt32(Int32 n = 0)
    {
        if (canRead(4))
        {
            n = BitConverter.ToInt32(_buffer, _nReadPos);
            Pop(4);
        }
        return n;
    }
    public Int64 ReadInt64(Int64 n = 0)
    {
        if (canRead(8))
        {
            n = BitConverter.ToInt64(_buffer, _nReadPos);
            Pop(8);
        }
        return n;
    }

    public byte ReadUInt8(byte n = 0)
    {
        if (canRead(1))
        {
            n = _buffer[_nReadPos];
            Pop(1);
        }
        return n;
    }
    public UInt16 ReadUInt16(UInt16 n = 0)
    {
        if (canRead(2))
        {
            n = BitConverter.ToUInt16(_buffer, _nReadPos);
            Pop(2);
        }
        return n;
    }
    public UInt32 ReadUInt32(UInt32 n = 0)
    {
        if (canRead(4))
        {
            n = BitConverter.ToUInt32(_buffer, _nReadPos);
            Pop(4);
        }
        return n;
    }
    public UInt64 ReadUInt64(UInt64 n = 0)
    {
        if (canRead(8))
        {
            n = BitConverter.ToUInt64(_buffer, _nReadPos);
            Pop(8);
        }
        return n;
    }

    public float ReadFloat(float n = 0.0f)
    {
        if (canRead(4))
        {
            n = BitConverter.ToSingle(_buffer, _nReadPos);
            Pop(4);
        }
        return n;
    }
    public double ReadDouble(double n = 0.0)
    {
        if (canRead(8))
        {
            n = BitConverter.ToDouble(_buffer, _nReadPos);
            Pop(8);
        }
        return n;
    }

    public string ReadString()
    {
        string str = string.Empty;
        int len = ReadInt32();
        if(canRead(len) && len > 0)
        {
            str = Encoding.UTF8.GetString(_buffer, _nReadPos, len);
            Pop(len);
        }
        return str;
    }
    public Int32[] ReadInt32Array()
    {
        int len = ReadInt32();
        if (len < 1)
        {
            //throw error log.
            return null;
        }
        Int32[] arr = new Int32[len];
        for (int n = 0;n < len; n++)
        {
            arr[n] = ReadInt32();
        }
        
        return arr;
    }

    public NetCMD ReadNetCmd()
    {
        return (NetCMD)ReadUInt16();
    }

    public void Release()
    {
    }

}

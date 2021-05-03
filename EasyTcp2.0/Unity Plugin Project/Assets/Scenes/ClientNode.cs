using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Globalization;

public class ClientNode : ClientBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        this.Create();
        string ip = "127.0.0.1";
        short port = 4567;
        if (this.Connect(ip, port))
            Debug.Log("Host<" + ip + ":" + port + "> connected.");

    }

    // Update is called once per frame
    void Update()
    {
        CellWriteStream sendStream = new CellWriteStream(1024);
        sendStream.setNetCmd(NetCMD.CMD_LOGOUT);

        sendStream.WriteInt8(5);
        sendStream.WriteInt16(6);
        sendStream.WriteInt32(7);
        sendStream.WriteInt64(8);

        sendStream.WriteFloat(14.0f);
        sendStream.WriteDouble(15.0);

        int[] pos = { 1, 2 };
        sendStream.WriteString("client.");
        sendStream.WriteInt32Arr(pos);

        int[] pos0 = { 5, 6, 7 };
        sendStream.WriteString("abc");
        sendStream.WriteInt32Arr(pos0);
        sendStream.finsh();
        this.SendData(sendStream);
        sendStream.Release();

        this.OnRun();
    }
    private void OnDestroy()
    {
        this.Close();
        Debug.Log("Disconnected.");
        Debug.Log("Exit.");
    }

    public override void OnNetMsgBytes(IntPtr data, int len)
    {
        CellReadStream recvStream = new CellReadStream(data,len);

        UInt16 t = recvStream.ReadUInt16();
        NetCMD t1 = recvStream.ReadNetCmd();
        sbyte a1 = recvStream.ReadInt8();
        Int16 a2 = recvStream.ReadInt16();
        Int32 a3 = recvStream.ReadInt32();
        Int64 a4 = recvStream.ReadInt64();
        float a5 = recvStream.ReadFloat();
        double a6 = recvStream.ReadDouble();

        string a7 = recvStream.ReadString(); 
        Int32[] a8 = recvStream.ReadInt32Array();
        
        string a9 = recvStream.ReadString();
        Int32[] a10 = recvStream.ReadInt32Array();

        recvStream.Release();
    }
}

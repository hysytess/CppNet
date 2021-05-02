using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;


public class DllTest : MonoBehaviour
{
    //[DllImport("EasyTcpDLLGo")]
    //public static extern int Add(int a, int b);
    //[DllImport("EasyTcpDLLGo")]
    //public static extern int Sub(int a, int b);

    [DllImport("EasyTcpClientPulgin")]
    public static extern int Add(int a, int b);
    [DllImport("EasyTcpClientPulgin")]
    public static extern int Sub(int a, int b);

    // 代理:向外交互 调用其他语言函数 回调
    public delegate void CallBack1(string s);

    // 将参数string s传给CallBack1 对象
    // 对应格式 void funName(stringType, functingPointor);
    [DllImport("EasyTcpDLLGo")]
    public static extern void TellCall1(string s, CallBack1 cb);

    [MonoPInvokeCallback(typeof(CallBack1))]
    public static void CallBackFun1(string s)
    {
        Debug.Log(s);
    }
    // Start is called before the first frame update
    void Start()
    {
        Debug.Log("1+2=" + Add(1, 2));
        Debug.Log("2-1=" + Sub(2, 1));
        TellCall1("C++&C# for unity!", CallBackFun1);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}

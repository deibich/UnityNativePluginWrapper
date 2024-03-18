using System.Runtime.InteropServices;
using UnityEngine;

public class TestLibWrapper : MonoBehaviour
{
    [DllImport("TestLib", EntryPoint = "TestCall")]
    private static extern void TestCall();

    // Start is called before the first frame update
    void Start()
    {
        #if UNITY_EDITOR
            UnityNativePluginWrapper.Instance.loadPlugin("Testlib.dll");
        #endif
    }

    private void OnDestroy()
    {
        #if UNITY_EDITOR
            UnityNativePluginWrapper.Instance.unloadPlugin("Testlib.dll");
        #endif
        
    }

    // Update is called once per frame
    private void Update()
    {
        TestCall();
    }
}

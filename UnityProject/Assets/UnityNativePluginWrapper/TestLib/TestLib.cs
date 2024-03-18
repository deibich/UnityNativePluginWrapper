using System.Runtime.InteropServices;
using UnityEngine;

public class TestLibWrapper : MonoBehaviour
{
    [DllImport("TestLib", EntryPoint = "TestCall")]
    private static extern void TestCall();

    void Awake()
    {
        #if UNITY_EDITOR
            // Make sure to call loadPlugin before using the plugin
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

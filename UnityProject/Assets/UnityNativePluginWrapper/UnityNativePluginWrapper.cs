using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using UnityEngine;

public class UnityNativePluginWrapper : MonoBehaviour
{

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void UnmanagedCharArrayToManagedDelegate(
    [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr, SizeParamIndex = 1)]
    string[] values,
    int valueCount);

    [DllImport("UnityNativePluginWrapper", EntryPoint = "UnloadAllPlugins")]
    private static extern void UnloadAllPlugins();

    [DllImport("UnityNativePluginWrapper", EntryPoint = "LoadPlugin")]
    private static extern bool LoadPlugin(string pluginName);

    [DllImport("UnityNativePluginWrapper", EntryPoint = "UnloadPlugin")]
    private static extern void UnloadPlugin(string pluginName);

    [DllImport("UnityNativePluginWrapper", EntryPoint = "AddLibrarySearchPath")]
    private static extern void AddLibrarySearchPath(string pluginName);

    [DllImport("UnityNativePluginWrapper", EntryPoint = "GetLoadedLibraryCount")]
    private static extern int GetLoadedLibraryCount();


    [DllImport("UnityNativePluginWrapper", EntryPoint = "GetLoadedLibraryNames", CallingConvention = CallingConvention.Cdecl)]
    private static extern void GetLoadedLibraryNames(UnmanagedCharArrayToManagedDelegate callback);


    public static UnityNativePluginWrapper Instance {
        get; private set;
    }


    string[] loadedLibraries = new string[] { };

    private void Awake()
    {
        if(Instance != null && Instance != this)
        {
            Destroy(this);
            return;
        }
        Instance = this;
        DontDestroyOnLoad(gameObject);
        addDllSearchPaths();
    }

    protected void addDllSearchPaths()
    {
        HashSet<string> dllDirsHashSet = new HashSet<string>();
        dllDirsHashSet.Add(Application.dataPath);

        DirectoryInfo di = new DirectoryInfo(Application.dataPath);
        FileInfo[] fileInfos = di.GetFiles("*.dll", SearchOption.AllDirectories);
        foreach (var fileInfo in fileInfos)
        {
            dllDirsHashSet.Add(fileInfo.Directory.FullName);
        }

        foreach (var dllDir in dllDirsHashSet)
        {
            AddLibrarySearchPath(dllDir.Replace("/", @"/"));
        }
    }

    protected void SetLoadedLibraryNames(string[] values, int valueCount)
    {
        loadedLibraries = values;
    }

    private void Update()
    {
        int currLibCount = GetLoadedLibraryCount();
        if (currLibCount != loadedLibraries.Length)
        {
            GetLoadedLibraryNames(SetLoadedLibraryNames);
        }
    }

    private void OnDisable()
    {
        UnloadAllPlugins();
    }

    public bool loadPlugin(string libraryName)
    {
        return LoadPlugin(libraryName);
    }

    public void unloadPlugin(string libraryName)
    {
        UnloadPlugin(libraryName);
    }
}

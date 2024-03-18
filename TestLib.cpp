#include "IUnityInterface.h"
#include "IUnityLog.h"

IUnityInterfaces* s_unityInterfaces{ nullptr };
IUnityLog* s_unityLogger{ nullptr };

extern "C" {
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		s_unityInterfaces = unityInterfaces;
		s_unityLogger = unityInterfaces->Get<IUnityLog>();

		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "TestLib loaded");
		}
	}
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
	{
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "TestLib unloaded");
		}
	}

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API TestCall()
	{
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "TestCall called!");
		}
	}
}

#include <string>
#include <iostream>
#include <vector>

#include <ranges>
#include <algorithm> 
#include <iterator>
#include <format>
#include <filesystem>

#include "IUnityInterface.h"
#include "IUnityLog.h"

#if _WINDOWS
#include <wtypes.h>
#endif

typedef void (UNITY_INTERFACE_API* UnityPluginLoadFct)(void* unityInferfaces);
typedef void (UNITY_INTERFACE_API* UnityPluginUnLoadFct)();

IUnityInterfaces* s_unityInterfaces{ nullptr };
IUnityLog* s_unityLogger{ nullptr };
std::vector<DLL_DIRECTORY_COOKIE> s_dllLibs{};

typedef void(*StringArrayDelegateFct)(const char* pStringValues[], int nValues);

class Library
{
public:
	Library(const std::string libraryNamePath) : libraryNamePath_(libraryNamePath) {}

	auto load(IUnityInterfaces* unityInterfaces) -> bool;
	auto unload() -> void;

	auto isLoaded() const -> bool;
	auto isPlugin() const -> bool;
	auto getName() const->std::string_view;


private:
	UnityPluginLoadFct unityPluginLoad_{ nullptr };
	UnityPluginUnLoadFct unityPluginUnload_{ nullptr };
	std::string libraryNamePath_{};

#if _WINDOWS
	HMODULE hGetProcIDDLL{ NULL };
#else
	void* hGetProcIDDLL{ nullptr };
#endif

};

auto Library::load(IUnityInterfaces* unityInterfaces) -> bool
{
	if (!isLoaded())
	{
#if _WINDOWS
		hGetProcIDDLL = LoadLibraryA(libraryNamePath_.c_str());
		if (hGetProcIDDLL == NULL)
		{
			hGetProcIDDLL = LoadLibraryExA(libraryNamePath_.c_str(), NULL, LOAD_LIBRARY_SEARCH_USER_DIRS);

		}
#else
		hGetProcIDDLL = dlopen(libraryNamePath_.c_str(), RTLD_LAZY);
#endif
	}
	if (isLoaded())
	{
		auto fctPointer = GetProcAddress(hGetProcIDDLL, "UnityPluginLoad");
		if (fctPointer != NULL)
		{
			unityPluginLoad_ = (UnityPluginLoadFct)fctPointer;
		}

		fctPointer = NULL;
		fctPointer = GetProcAddress(hGetProcIDDLL, "UnityPluginUnload");
		if (fctPointer != NULL)
		{
			unityPluginUnload_ = (UnityPluginUnLoadFct)fctPointer;
		}

		if (isPlugin())
		{
			unityPluginLoad_(unityInterfaces);
		}
		else
		{
			unityPluginLoad_ = NULL;
			unityPluginUnload_ = NULL;
		}
	}
	return isLoaded();
}

auto Library::unload() -> void
{
	if (isLoaded())
	{
		if (isPlugin())
		{
			unityPluginUnload_();
		}
#if _WINDOWS

		FreeLibrary(hGetProcIDDLL);
		hGetProcIDDLL = NULL;
#else
		dlclose(hGetProcIDDLL)
			hGetProcIDDLL = nullptr;
#endif
	}
}

auto Library::isLoaded() const -> bool
{
#if _WINDOWS
	return hGetProcIDDLL != NULL;
#else
	return hGetProcIDDLL != nullptr;
#endif
}

auto Library::isPlugin() const -> bool
{
#if _WINDOWS
	return unityPluginLoad_ != NULL && unityPluginUnload_ != NULL;
#else
	return unityPluginLoad_ != nullptr && unityPluginUnload_ != nullptr;
#endif
}

auto Library::getName() const -> std::string_view
{
	return libraryNamePath_;
}

class LibraryCollection {

public:
	LibraryCollection(LibraryCollection& other) = delete;
	void operator=(const LibraryCollection&) = delete;

	static auto getInstance() -> LibraryCollection& {
		static LibraryCollection instance;
		return instance;
	};

	auto loadLibrary(std::string& libraryNamePath) -> bool;
	auto unloadLibrary(std::string& libraryNamePath) -> bool;
	auto unloadAll() -> void;
	auto getLoadedLibraryNames() -> std::vector<std::string>;
	auto getLoadedLibraryCount() -> int { return loadedLibraries_.size(); };
private:
	LibraryCollection() {};

	std::vector<Library> loadedLibraries_{};
};

auto LibraryCollection::getLoadedLibraryNames() -> std::vector<std::string>
{
	std::vector<std::string> libraryNames{};
	for (const auto& lib : loadedLibraries_)
	{
		libraryNames.push_back(std::string{ lib.getName() });
	}
	return libraryNames;
}

auto LibraryCollection::loadLibrary(std::string& libraryNamePath) -> bool
{
	if (auto result = std::find_if(loadedLibraries_.begin(), loadedLibraries_.end(), [&libraryNamePath](const auto& containedLib) {return libraryNamePath == containedLib.getName(); }) != loadedLibraries_.end())
	{
		return true;
	}

	Library newLib{ libraryNamePath };
	if (!newLib.load(s_unityInterfaces))
	{
		return false;
	}

	loadedLibraries_.push_back(newLib);
	return true;
}

auto LibraryCollection::unloadLibrary(std::string& libraryNamePath) -> bool
{
	auto result = std::find_if(loadedLibraries_.begin(), loadedLibraries_.end(), [&libraryNamePath](const auto& containedLib) {return libraryNamePath == containedLib.getName(); });
	if (result == loadedLibraries_.end())
	{
		return true;
	}
	result->unload();
	if (result->isLoaded())
	{
		// Could not unload library.
		return false;
	}
	else
	{
		loadedLibraries_.erase(result);
		return true;
	}
}

auto LibraryCollection::unloadAll() -> void
{
	auto loadedLibraryIterator = loadedLibraries_.begin();
	while (loadedLibraryIterator != loadedLibraries_.end()) {
		loadedLibraryIterator->unload();
		if (loadedLibraryIterator->isLoaded())
		{
			// Could not unload library.
			loadedLibraryIterator++;
		}
		else
		{
			loadedLibraryIterator = loadedLibraries_.erase(loadedLibraryIterator);
		}
	}
}

extern "C" {

	// If exported by a plugin, this function will be called when the plugin is loaded.
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
	{
		s_unityInterfaces = unityInterfaces;
		s_unityLogger = unityInterfaces->Get<IUnityLog>();

		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "Wrapper loaded");
		}
	}

	// If exported by a plugin, this function will be called when the plugin is about to be unloaded.
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
	{
		LibraryCollection::getInstance().unloadAll();
		// Probably never called in Editor mode.
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "Wrapper unloaded");
		}
	}

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnloadAllPlugins()
	{
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, "Unload all plugins");
		}
		LibraryCollection::getInstance().unloadAll();
	}

	bool UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API LoadPlugin(char const* libraryNamePath)
	{
		std::string libName{ libraryNamePath };
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, std::format("Try to load {}", libName).c_str());
		}
		bool loaded = LibraryCollection::getInstance().loadLibrary(libName);
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, std::format("{} loaded {}", libName, loaded).c_str());
		}
		return loaded;
	}

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnloadPlugin(char const* libraryNamePath)
	{
		std::string libName{ libraryNamePath };
		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, std::format("Try to unload {}", libName).c_str());
		}
		LibraryCollection::getInstance().unloadLibrary(libName);
	}

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API AddLibrarySearchPath(char const* librarySearchPath)
	{
		std::filesystem::path p = librarySearchPath;
		if (!std::filesystem::is_directory(p) || !std::filesystem::exists(p))
		{
			if (s_unityLogger != nullptr)
			{
				UNITY_LOG(s_unityLogger, std::format("dll search path {} does not exist or is not a directory", p.string()).c_str());
			}
		}

		if (s_unityLogger != nullptr)
		{
			UNITY_LOG(s_unityLogger, std::format("Add {} to dll search path", librarySearchPath).c_str());
		}

		auto addedPath = AddDllDirectory(p.c_str());
		if (addedPath != NULL) {
			s_dllLibs.push_back(addedPath);
		}
	}

	int UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetLoadedLibraryCount()
	{
		return LibraryCollection::getInstance().getLoadedLibraryCount();
	}

	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetLoadedLibraryNames(StringArrayDelegateFct callback)
	{
		if (callback == nullptr)
		{
			return;
		}

		auto libNames = LibraryCollection::getInstance().getLoadedLibraryNames();

		std::vector<const char*>  vc;
		std::transform(libNames.begin(), libNames.end(), std::back_inserter(vc), [](const std::string& s) {return s.c_str(); });

		callback(vc.data(), vc.size());
	}
}

/*
File:   win32_appLoading.cpp
Author: Taylor Robbins
Date:   06\06\2017
Description: 
	** Contains functions and types related to loading the application DLL

#included from win32_main.cpp 
*/

//+================================================================+
//|                           Structs                              |
//+================================================================+
struct LoadedApp_t
{
	bool isValid;
	Version_t version;
	HMODULE module;
	FILETIME lastWriteTime;
	
	AppGetVersion_f*      AppGetVersion;
	AppReloading_f*       AppReloading;
	AppReloaded_f*        AppReloaded;
	AppInitialize_f*      AppInitialize;
	AppUpdate_f*          AppUpdate;
	AppClosing_f*         AppClosing;
};

//+================================================================+
//|                       Stub Functions                           |
//+================================================================+
AppGetVersion_DEFINITION(AppGetVersion_Stub)
{
	Version_t version = { 0, 0, 0 };
	return version;
}
AppInitialize_DEFINITION(AppInitialize_Stub)
{
	
}
AppReloading_DEFINITION(AppReloading_Stub)
{
	
}
AppReloaded_DEFINITION(AppReloaded_Stub)
{
	
}
AppUpdate_DEFINITION(AppUpdate_Stub)
{
	
}
AppClosing_DEFINITION(AppClosing_Stub)
{
	
}


//+================================================================+
//|                       Main Functions                           |
//+================================================================+
bool LoadDllCode(const char* appDllName, const char* tempDllName, LoadedApp_t* loadedApp)
{
	ClearPointer(loadedApp);
	
	loadedApp->lastWriteTime = GetFileWriteTime(appDllName);
	
	#if DEBUG
		u32 copyTries = 0;
		while (!CopyFileA(appDllName, tempDllName, false))
		{
			DWORD error = GetLastError();
			if (error != ERROR_SHARING_VIOLATION)
			{
				Win32_PrintLine("CopyFileA error: %u", GetLastError());
				copyTries++;
				
				if (copyTries >= 100) 
				{
					Win32_WriteLine("Could not copy DLL.");
					return false;
				}
			}
		}
		// Win32_PrintLine("Tried to copy %u times", copyTries);
		loadedApp->module = LoadLibraryA(tempDllName);
	#else
		loadedApp->module = LoadLibraryA(appDllName);
	#endif
	
	
	if (loadedApp->module != 0)
	{
		loadedApp->AppGetVersion = (AppGetVersion_f*) GetProcAddress(loadedApp->module, "App_GetVersion");
		loadedApp->AppInitialize = (AppInitialize_f*) GetProcAddress(loadedApp->module, "App_Initialize");
		loadedApp->AppReloading  = (AppReloading_f*)  GetProcAddress(loadedApp->module, "App_Reloading");
		loadedApp->AppReloaded   = (AppReloaded_f*)   GetProcAddress(loadedApp->module, "App_Reloaded");
		loadedApp->AppUpdate     = (AppUpdate_f*)     GetProcAddress(loadedApp->module, "App_Update");
		loadedApp->AppClosing    = (AppClosing_f*)    GetProcAddress(loadedApp->module, "App_Closing");
		
		loadedApp->isValid = (
			loadedApp->AppGetVersion != nullptr &&
			loadedApp->AppInitialize != nullptr &&
			loadedApp->AppReloading != nullptr &&
			loadedApp->AppReloaded != nullptr &&
			loadedApp->AppUpdate != nullptr &&
			loadedApp->AppClosing != nullptr
		);
	}
	
	if (!loadedApp->isValid)
	{
		loadedApp->AppGetVersion = AppGetVersion_Stub;
		loadedApp->AppInitialize = AppInitialize_Stub;
		loadedApp->AppReloading  = AppReloading_Stub;
		loadedApp->AppReloaded   = AppReloaded_Stub;
		loadedApp->AppUpdate     = AppUpdate_Stub;
		loadedApp->AppClosing    = AppClosing_Stub;
	}
	
	loadedApp->version = loadedApp->AppGetVersion(nullptr);
	
	return loadedApp->isValid;
}

void FreeDllCode(LoadedApp_t* loadedApp)
{
	if (loadedApp != nullptr &&
		loadedApp->module != 0)
	{
		bool32 freeResult = FreeLibrary(loadedApp->module);
		Assert(freeResult);
	}
	
	ClearPointer(loadedApp);
}


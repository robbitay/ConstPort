/*
File:   osx_appLoading.cpp
Author: Taylor Robbins
Date:   10\08\2017
Description: 
	** Holds all the functions that are used to load and lookup functions in the application DLL 
*/

struct LoadedApp_t
{
	bool isValid;
	bool reinitializeApp;
	Version_t version;
	void* handle;
	time_t lastWriteTime;
	
	AppGetVersion_f*      GetVersion;
	AppReloaded_f*        Reloaded;
	AppInitialize_f*      Initialize;
	AppUpdate_f*          Update;
	AppGetSoundSamples_f* GetSoundSamples;
	AppClosing_f*         Closing;
};

// +--------------------------------------------------------------+
// |                      App Stub Functions                      |
// +--------------------------------------------------------------+
AppGetVersion_DEFINITION(AppGetVersion_Stub)
{
	Version_t result = {};
	return result;
}
AppReloaded_DEFINITION(AppReloaded_Stub)
{
	
}
AppInitialize_DEFINITION(AppInitialize_Stub)
{
	
}
AppUpdate_DEFINITION(AppUpdate_Stub)
{
	
}
AppGetSoundSamples_DEFINITION(AppGetSoundSamples_Stub)
{
	
}
AppClosing_DEFINITION(AppClosing_Stub)
{
	
}

// +--------------------------------------------------------------+
// |                       Public Functions                       |
// +--------------------------------------------------------------+
time_t GetLastModifiedTime(const char* filePath)
{
	struct stat fileAttributes;
	stat(filePath, &fileAttributes);
	return fileAttributes.st_mtime;
}

bool LoadDllCode(const char* appDllPath, const char* tempDllPath, LoadedApp_t* loadedApp)
{
	ClearPointer(loadedApp);
	
	loadedApp->lastWriteTime = GetLastModifiedTime(appDllPath);
	
	#if DEBUG
	{
		char copyCommand[256] = {};
		snprintf(copyCommand, sizeof(copyCommand), "cp %s %s", appDllPath, tempDllPath);
		system(copyCommand);
		
		loadedApp->handle = dlopen(tempDllPath, RTLD_NOW);
	}
	#else
	{
		loadedApp->handle = dlopen(appDllPath, RTLD_NOW);
	}
	#endif
	
	if (loadedApp->handle != nullptr)
	{
		loadedApp->GetVersion      = (AppGetVersion_f*)      dlsym(loadedApp->handle, "App_GetVersion");
		loadedApp->Initialize      = (AppInitialize_f*)      dlsym(loadedApp->handle, "App_Initialize");
		loadedApp->Reloaded        = (AppReloaded_f*)        dlsym(loadedApp->handle, "App_Reloaded");
		loadedApp->Update          = (AppUpdate_f*)          dlsym(loadedApp->handle, "App_Update");
		loadedApp->GetSoundSamples = (AppGetSoundSamples_f*) dlsym(loadedApp->handle, "App_GetSoundSamples");
		loadedApp->Closing         = (AppClosing_f*)         dlsym(loadedApp->handle, "App_Closing");
		
		loadedApp->isValid = (
			loadedApp->GetVersion != nullptr &&
			loadedApp->Initialize != nullptr &&
			loadedApp->Reloaded != nullptr &&
			loadedApp->Update != nullptr &&
			loadedApp->GetSoundSamples != nullptr &&
			loadedApp->Closing != nullptr
		);
	}
	
	if (loadedApp->isValid == false)
	{
		loadedApp->GetVersion =      AppGetVersion_Stub;
		loadedApp->Initialize =      AppInitialize_Stub;
		loadedApp->Reloaded =        AppReloaded_Stub;
		loadedApp->Update =          AppUpdate_Stub;
		loadedApp->GetSoundSamples = AppGetSoundSamples_Stub;
		loadedApp->Closing =         AppClosing_Stub;
	}
	
	loadedApp->version = loadedApp->GetVersion(&loadedApp->reinitializeApp);
	
	return loadedApp->isValid;
}

void FreeDllCode(LoadedApp_t* loadedApp)
{
	if (loadedApp != nullptr &&
		loadedApp->handle != nullptr)
	{
		dlclose(loadedApp->handle);
	}
	
	ClearPointer(loadedApp);
}

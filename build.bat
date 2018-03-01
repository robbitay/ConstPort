@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

set TimeString=%date:~-4,4%%date:~-10,2%%date:~-7,2%%time:~0,2%%time:~3,2%%time:~6,2%

rem mkdir build > NUL 2> NUL
rem echo Running from %cd%
rem echo Time is %TimeString%

set CompilePlatform=1
set CompileApplication=1
set DebugBuild=1
set ProjectName=ConstPort

REM The boost regex library should be built using the b2 tool that comes
REM with the Boost distribution with the following command:
REM 	b2 regex runtime-debugging=on variant=debug runtime-link=static link=static threading=multi address-model=64
REM 	b2 regex runtime-debugging=off variant=release runtime-link=static link=static threading=multi address-model=64
set BoostVersion=1_65_0
set BoostLibSuffix=1_65

if "%DebugBuild%"=="1" (
	set DebugDependantFlags=/MTd -DDEBUG=1
	set DebugDependantPaths=/LIBPATH:"..\lib\debug"
	set DebugDependantLibraries=glew32d.lib libboost_regex-vc140-mt-sgd-%BoostLibSuffix%.lib
) else (
	set DebugDependantFlags=/MT -DDEBUG=0
	set DebugDependantPaths=/LIBPATH:"..\lib\release"
	set DebugDependantLibraries=glew32.lib libboost_regex-vc140-mt-s-%BoostLibSuffix%.lib
)

set Definitions=-DWINDOWS_COMPILATION -DDOUBLE_RESOLUTION=0 -DDOUBLE_MOUSE_POS=0
set IgnoredWarnings=/wd4201 /wd4100 /wd4189 /wd4996 /wd4127 /wd4505 /wd4101 /wd4702 /wd4458 /wd4324
set IncludeDirectories=/I"W:\lib\mylib" /I"W:\lib\glew-2.0.0\include" /I"W:\lib\glfw-3.2.1\include" /I"W:\lib\stb" /I"W:\lib\jsmn" /I"W:\lib\boost_%BoostVersion%"
set CompilerFlags=/FC /Zi /EHsc /nologo /GS- /Gm- -GR- /EHa- /Fm /Od /Oi /WX /W4 %DebugDependantFlags% %Definitions% %IgnoredWarnings% %IncludeDirectories%

set LibraryDirectories=%DebugDependantPaths%
set Libraries=gdi32.lib User32.lib Shell32.lib opengl32.lib glfw3.lib Shlwapi.lib Advapi32.lib %DebugDependantLibraries%
set LinkerFlags=-incremental:no %LibraryDirectories% %Libraries%

set AppExports=/EXPORT:App_GetVersion /EXPORT:App_Initialize /EXPORT:App_Reloading /EXPORT:App_Reloaded /EXPORT:App_Update /EXPORT:App_Closing

rem echo [Building...]

REM Compile the resources file to generate resources.res which defines our program icon
rc /nologo ..\code\resources.rc

del *.pdb > NUL 2> NUL

if "%CompilePlatform%"=="1" (
	echo[
	
	python ..\IncrementVersionNumber.py ..\code\win32_version.h
	
	cl /Fe%ProjectName%.exe %CompilerFlags% %IncludeDirectories% ..\code\win32_main.cpp /link %LinkerFlags% kernel32.lib ..\code\resources.res

	if "%ERRORLEVEL%"=="0" (
		echo [Platform Build Succeeded!]
	) else (
		echo [Platform Build Failed! %ERRORLEVEL%]
	)
)

if "%CompileApplication%"=="1" (
	echo[
	
	python ..\IncrementVersionNumber.py ..\code\appVersion.h
	
	cl /Fe%ProjectName%.dll %CompilerFlags% %IncludeDirectories% ..\code\app.cpp /link %LinkerFlags% %AppExports% /DLL /PDB:"%ProjectName%_%TimeString%.pdb"

	if "%ERRORLEVEL%"=="0" (
		echo [Application Build Succeeded!]
	) else (
		echo [Application Build Failed! %ERRORLEVEL%]
	)
)

echo[

REM Delete the object files that were created during compilation
del *.obj > NUL 2> NUL

rem echo [Done!]
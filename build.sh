#!/bin/bash

ProjectName="ConstPort"
CompilePlatform=1
CompileApplication=1
DebugBuild=0

if [ $DebugBuild -gt 0 ]
then
	DebugDepCompilerFlags="-DDEBUG=1"
	DebugDepLibDirs="-L../../../lib/glfw/build/src/Debug -L../../../lib/boost_1_65_1/bin.v2/libs/regex/build/darwin-4.2.1/debug/link-static/runtime-link-static/threading-multi"
else
	DebugDepCompilerFlags="-DDEBUG=0"
	DebugDepLibDirs="-L../../../lib/glfw/build/src/Release -L../../../lib/boost_1_65_1/bin.v2/libs/regex/build/darwin-4.2.1/release/link-static/runtime-link-static/threading-multi"
fi

PlatformMainFile="../code/osx_main.cpp"
ApplicationMainFile="../code/app.cpp"
CompilerFlags="-g -DOSX_COMPILATION=1 -DDOUBLE_RESOLUTION=0"
LinkerFlags="-std=gnu++0x"
IncludeDirectories="-I../../../lib/mylib -I../../../lib/glfw/include -I../../../lib/glew/include -I../../../lib/stb -I../../../lib/jsmn -I../../../lib/boost_1_65_1/"
LibraryDirectories=" -L../../../lib/glew/lib"
Libraries="-lglfw3 -lglew -lboost_regex"
Frameworks="-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"

if [ $CompilePlatform -gt 0 ]
then
	python ../IncrementVersionNumber.py ../code/osx_version.h
	
	# echo [Building OSX Platform]
	g++ $CompilerFlags $DebugDepCompilerFlags $PlatformMainFile -o $ProjectName $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags $DebugDepLibDirs
	
	if [ $? -ne 0 ]
	then
	    echo Platform Build Failed!
	else
		echo Platform Build Succeeded!
	fi
	
	echo 
fi

if [ $CompileApplication -gt 0 ]
then
	python ../IncrementVersionNumber.py ../code/appVersion.h
	
	# echo [Building OSX Application]
	g++ -shared -fvisibility=hidden $CompilerFlags $DebugDepCompilerFlags $ApplicationMainFile -o $ProjectName.dll $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags $DebugDepLibDirs
	
	if [ $? -ne 0 ]
	then
	    echo Application Build Failed!
	else
		echo Application Build Succeeded!
	fi
	
	echo
fi
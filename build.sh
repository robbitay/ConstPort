#!/bin/bash

ProjectName="ConstPort"
CompilePlatform=0
CompileApplication=1

PlatformMainFile="../code/osx_main.cpp"
ApplicationMainFile="../code/app.cpp"
CompilerFlags="-g -DDEBUG=1 -DOSX_COMPILATION=1 -fvisibility=hidden"
LinkerFlags="-std=gnu++0x"
IncludeDirectories="-I../../../lib/mylib -I../../../lib/glfw/include -I../../../lib/glew/include -I../../../lib/stb -I../../../lib/jsmn -I../../../lib/boost_1_65_1/"
LibraryDirectories="-L../../../lib/glfw/build/src/Debug -L../../../lib/glew/lib -L../../../lib/boost_1_65_1/bin.v2/libs/regex/build/darwin-4.2.1/debug/link-static/runtime-link-static/threading-multi"
Libraries="-lglfw3 -lglew -lboost_regex"
Frameworks="-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"

if [ $CompilePlatform -gt 0 ]
then
	echo [Building OSX Platform]
	g++ $CompilerFlags $PlatformMainFile -o $ProjectName $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags
	
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
	echo [Building OSX Application]
	g++ -shared $CompilerFlags $ApplicationMainFile -o $ProjectName.dll $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags
	
	if [ $? -ne 0 ]
	then
	    echo Application Build Failed!
	else
		echo Application Build Succeeded!
	fi
	
	echo
fi
#!/bin/bash

ProjectName="ConstPort"
CompilePlatform=1
CompileApplication=1
CreateApplicationBundle=1
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
CompilerFlags="-g -DOSX_COMPILATION -DDOUBLE_RESOLUTION=1 -DDOUBLE_MOUSE_POS=1 -Wno-format-security"
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

	cp $ProjectName ../data/$ProjectName
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

	cp $ProjectName.dll ../data/$ProjectName.dll
fi

if [ $CreateApplicationBundle -gt 0 ]
then
	echo [Creating application bundle]
	
	cd ../data
	
	rm -f -r ConstPort.app
	mkdir ConstPort.app
	mkdir ConstPort.app/Contents
	cp Info.plist ConstPort.app/Contents/Info.plist

	mkdir ConstPort.app/Contents/MacOS
	cp ConstPort     ConstPort.app/Contents/MacOS/ConstPort
	cp ConstPort.dll ConstPort.app/Contents/MacOS/ConstPort.dll
	cp glew32.dll    ConstPort.app/Contents/MacOS/glew32.dll
	
	cp -r Resources  ConstPort.app/Contents/MacOS/Resources
	
	mkdir ConstPort.app/Contents/Resources
	cp Icon.icns ConstPort.app/Contents/Resources/Icon.icns
	
	echo Application Bundle Succeeded!
	
	echo 
fi

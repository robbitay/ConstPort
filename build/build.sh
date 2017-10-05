#!/bin/bash

echo "OSX Build!"

ExecutableName="ConstPort"
PlatformMainFile="../code/osx_main.cpp"
CompilerFlags=""
LinkerFlags="-std=gnu++0x"
IncludeDirectories="-I../../../lib/glfw/include -I../../../lib/glew/include"
LibraryDirectories="-L../../../lib/glfw/build/src/Debug -L../../../lib/glew/lib"
Libraries="-lglfw3 -lglew"
Frameworks="-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"

gcc $CompilerFlags $PlatformMainFile -o $ExecutableName $IncludeDirectories $LibraryDirectories $Libraries $Frameworks $LinkerFlags

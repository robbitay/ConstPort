This software is released under the MIT License

Copyright (c) 2017 Taylor Robbins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

# Dependencies:
	Currently Const Port depends on GLFW and GLEW to compile and run. 
To compile the project you must first compile both GLFW and GLEW and
then modify the build/build.bat file to point to the directory where
the .lib files are found. 
	The built executable must be run from the data/ folder alongside 
copies of the GLFW and GLEW dlls.

# Installation:
	Simply extract one of the release archives found in the Release/ 
folder into a directory and run ConstPort.exe.

# Compilation:
	Compilation on Windows is performed by running the build/build.bat file
from the base directory of the project. 
	The build.bat has some hard coded paths that must be changed in order 
for the project to compile correctly. You must have a version of Microsoft 
Visual Studios C++ Compiler installed and the path in build.bat should match 
that of the vcvarsall.bat that comes installed with it. 
	The build file also has hard coded relative paths to where it should find 
the GLFW and GLEW library and headers files.
	The build.bat file that comes with the repository is configured to build
in Debug x64 mode using the Debug x64 versions of GLFW and GLEW. Various
changes need to be made to build.bat to compile a release version, however most
of the debug behavior can be toggled on/off by changing -DDEBUG=1 to -DDEBUG=0


# Controls
Ctrl + A:         Select/Deselect All
Ctrl + C:         Copy selected text to clipboard
Ctrl + Shift + C: Clear console of all text
Ctrl + O:         Show the COM menu
Ctrl + [1-9]:     Connected to COM port that is at a specific index in the list of available COM ports
Home:             Goto to beginning
End:              Goto end
Page Up:          Move up a page
Page Down:        Move down a page
Up/Down Arrows:   Move the view up/down (Shift to increase speed)
Scroll Wheel:     Move the view up/down
Hold Cntrl:       Shows receive time of line being hovered over (Hold shift for alternate format)

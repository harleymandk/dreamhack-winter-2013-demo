Demo : From DK With Love
===================================
Final Combo/Windows version : https://demozoo.org/productions/94750/

**ToDO:**  
-----------------------------------
- pænere splash logo.... hvad skal der stå???
- fix Skybox  - Passer ikke sammen... google?
- Musik. Skal den ind i 64K skal musikken fylde mindre...  (BASS kan spille : xm , IT og mod filer!)--
- Noget andet end ringe...   kode/ideer modtages ;-)
- Afsnit med Shaders? (how to do???)
- musik kunne pakkes med : Clinkster ????

**Comments:**
--------------------------------------------------
For at bruge denne demo skal du have :
Visual Studio v10 eller højere med C/c++ 
OpenGL
GLEW (The OpenGL Extension Wrangler Library )
BASS.dll  (musik) 

WinAmp til hører .xm musik ;-)
EXE filen pakkes med :  kkrunchy.exe "From Dk With Love.exe"

**GLUT guide:**
--------------------------------------------------
Source : http://www.cs.uiowa.edu/~cwyman/classes/common/howto/winGLUT.html
Compiling OpenGL Progams at Home Using Visual Studio
Windows does not include GLUT standard, like the lab machines in MLH 301 do. Thus, getting
your OpenGL programs to compile and run at home is slightly more difficult. However by
following the following steps, you should be able to figure out how to make it work: Download GLUT

Unzip the file.

Put the file "glut32.dll" into the system path.
This can be in the same directory as your executable file.
On Windows XP or earlier, this can be in "C:\WINDOWS\system32"
Or you can create a directory like "C:\DLLs", put the file in this directory and change your system
path to include this new directory.
Do this by opening Control Panel -> System, clicking on "Advanced System Settings", followed by "Environment Variables", and editing the "Path" variable.
Put the file "glut.h" into the standard Visual C++ include directoy
(For Visual Studio 2010, this should be: "C:\Program Files\Microsoft SDKs\Windows\v7.0A\include\gl")
(For Visual Studio 2008, this should be: "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\gl")
(For Visual Studio 2005, this should be: "C:\Program Files\Microsoft Visual Studio.NET\Vc7\PlatformSDK\Include\gl")
You've got the right directory if you see a copy of "gl.h"
Put the file "glut32.lib" into the standard Visual C++ library directory
(For Visual Studio 2010, this should be: "C:\Program Files\Microsoft SDKs\Windows\v7.0A\Lib")
(For Visual Studio 2008, this should be: "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib")
(For Visual Studio 2005, this should be: "C:\Program Files\Microsoft Visual Studio.NET\Vc7\PlatformSDK\lib")
There should be lots of .lib files here, including "opengl32.lib" and "glu32.lib".

Make sure your Visual C++ project links in the GLUT/gl/glu libraries (see also this page). This is located in:

Menu: "Project -> (your-project-name) Properties"
Tab: "Configuration Properties -> Linker -> Input"
Under "Additional Dependancies", add "glut32.lib glu32.lib opengl32.lib"
#include < GL/glut.h > in your program.
Note: This needs to come after you #include < stdio.h > and < stdlib.h >.
Note: This needs to come before you #include < GL/gl.h >.
Also note that glut.h includes gl.h for you (so you need not explicitly #include < GL/gl.h >).
You should not include windows.h or any other Windows-specific header files, no matter what you
may read on random forum postings!
If you get compilation errors because of multiple conflicting definitions of "exit()", then "stdio.h"
and "glut.h" have been #include'd in the wrong order. You may fix this by:
Reordering your #include files (see step #7). This is the "right" way.
Add "#define GLUT_DISABLE_ATEXIT_HACK" to glut.h on the line immediately after the first 1
"#if defined(_WIN32)".
If you happen to have a 64-bit version of Windows and Visual Studio, make sure you compile a 32-
bit executable.

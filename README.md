# UTeM Main Campus 3D Walkthrough

## Overview
This project develops a 3D walkthrough simulation of the FTMK faculty building located at UTeM Main Campus. The simulation features an immersive first-person perspective, collision detection, and ambient background music to provide a realistic exploration experience.

## Tech Stack
- OpenGL
- C++
- Visual Studio

## Controls
- w / W = Move forward
- a / A = Move left
- d / D = Move right
- s / S = Move back
- q / Q = Yaw left
- e / E = Yaw right
- f / F = Toggle full screen
- b / B = Toggle collision box
- Mouse drag = Change point of view

## Setup 
1. Download Visual Studio: https://visualstudio.microsoft.com/vs/
2. Install Desktop development with C++ in Visual Studio
3. Download glutdlls37beta.zip : https://www.opengl.org/resources/libraries/glut/glut_downloads.php
4. Extract the ZIP file and copy the required files to the following locations  
   **(Adjust the year in paths based on your Visual Studio version, 2022 is used as an example):**
   - glut.h > C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\include\gl
   - glut.lib >  C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\lib\x64
   - glut32.lib > C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\lib\x86
   - glut.dll and glut32.dll >  C:\Windows\SysWOW64
   - glut32.dll > C:\Windows\System32
5. Get `freeglut` from NuGet Package Manager
   
## Demo 
Demo video available on YouTube: `https://youtu.be/gv1D1eKbUqQ`

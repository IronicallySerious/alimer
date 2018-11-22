@echo off
setlocal
set "SOURCE=%~dp0"
set "SOURCE=%SOURCE:~0,-1%"
set "BUILD=build/vs2017_win64_ninja"
cmake -E make_directory "%BUILD%" && cmake -E chdir "%BUILD%" cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="Alimer-SDK" "%SOURCE%"

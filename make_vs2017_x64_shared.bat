@echo off
setlocal
set "SOURCE=%~dp0"
set "SOURCE=%SOURCE:~0,-1%"
set "BUILD=build/vs2017_win64"
cmake -E make_directory "%BUILD%" && cmake -E chdir "%BUILD%" cmake -G "Visual Studio 15 2017 Win64" -DALIMER_SHARED=ON -DCMAKE_SYSTEM_VERSION=10.0.16299.0 -DCMAKE_INSTALL_PREFIX="Alimer-SDK" "%SOURCE%"

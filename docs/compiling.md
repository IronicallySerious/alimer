# Compiling

## Quick start

Use the guide below to get alimer up and running as quickly as possible. Scroll further below for advanced build options that allow for more customization.

- Install git (https://git-scm.com) and CMake 3.5.0 or higher (https://cmake.org)
  - Ensure they are added to your *PATH* environment variable
- Install other dependencies
  - See [here](#otherDeps)
- Run the following commands in the terminal/command line:
  - `git clone https://github.com/amerkoleci/alimer.git`
  - `cd alimer`
  - `mkdir build`
  - `cd build`
  - `cmake -G "$generator$" ..`
    - Where *$generator$* should be replaced with any of the supported generators. Some common ones:
	  - `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
	  - `Unix Makefiles`
	  - `Ninja`
	  - `Xcode`
	  - See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
	- Optionally provide the `CMAKE_INSTALL_PREFIX` to override the default install path:
	  - `cmake -G "$generator$" -DCMAKE_INSTALL_PREFIX=/path/to/install ..`
  - `cmake --build . --config Release`
    - Alternatively you can also run your build system manually (depending on your choice of generator):
	  - Visual Studio solution is present at `alimer/build/Alimer.sln`
	  - XCode project is present at `alimer/build`
	  - Makefiles are present at `alimer/build`
  - `cmake --build . --config Release --target install`
	- Alternatively you can run the install target in your chosen build tool
    - Note that files install to the default install folder, unless you have overriden it as specified above


## <a name="otherDeps"></a>Other dependencies

The following dependencies will need to be installed manually.

**Windows**
  - **Windows SDK 10.0.17763.0** 
	- Requred when compiling D3D12, D3D11 or XAudio backend.
  - **DirectX Debug Layer** (Required by default on Windows 10)
    - Necessary when you have D3D12 or D3D11 backend enabled
    - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"

**Linux**
  - **OpenGL**
    - Required by default, but optional if you have chosen a different RenderAPI in *CMake* options
    - Debian/Ubuntu: `apt-get install libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev`
  - **X11**
    - Debian/Ubuntu: `apt-get install libx11-dev libxcursor-dev libxrandr-dev libxi-dev`
  - **LibUUID**
    - Debian/Ubuntu: `apt-get install uuid-dev`
  - **LibICU**
    - Debian/Ubuntu: `apt-get install libicu-dev`

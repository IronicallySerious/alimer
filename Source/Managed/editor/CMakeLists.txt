# CMake install has issues with new C# project format.
add_target_csharp(Alimer.Studio ${CMAKE_CURRENT_SOURCE_DIR}/Alimer.Studio/Alimer.Studio.csproj)
set_target_properties(Alimer.Studio PROPERTIES FOLDER Managed)

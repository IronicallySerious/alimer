# CMake install has issues with new C# project format.
add_target_csharp(AlimerSharp ${CMAKE_CURRENT_SOURCE_DIR}/AlimerSharp/AlimerSharp.csproj)
set_target_properties(AlimerSharp PROPERTIES FOLDER Managed)

add_dependencies(AlimerSharp libalimerc)

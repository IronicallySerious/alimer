#
# Copyright (c) 2018 Amer Koleci and contributors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Replace compilation flags, configuration type is optional
macro(replace_compile_flags search replace)
	set(MacroArgs "${ARGN}")
	if( NOT MacroArgs )
		string(REGEX REPLACE "${search}" "${replace}" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
		string(REGEX REPLACE "${search}" "${replace}" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
        foreach(MacroArg IN LISTS MacroArgs)
            string(TOUPPER ${MacroArg} CONFIG)
            string(REGEX REPLACE "${search}" "${replace}" CMAKE_C_FLAGS_${CONFIG} "${CMAKE_C_FLAGS_${CONFIG}}")
            string(REGEX REPLACE "${search}" "${replace}" CMAKE_CXX_FLAGS_${CONFIG} "${CMAKE_CXX_FLAGS_${CONFIG}}")
        endforeach()
	endif()
endmacro()

# Set each source file proper source group
macro(set_source_groups filePaths)
	foreach(filePath ${filePaths})
		get_filename_component(dir_name ${filePath} DIRECTORY)
		if( NOT "${dir_name}" STREQUAL "" )
			string(REGEX REPLACE "[.][.][/]" "" GroupName "${dir_name}")
			string(REGEX REPLACE "/" "\\\\" GroupName "${GroupName}")
			source_group("${GroupName}" FILES ${filePath})
		else()
			source_group("" FILES ${filePath})
		endif()
	endforeach()
endmacro()

# Get all source files recursively and add them to pResult
macro(alimer_find_source_files pResult)
	set(FileList)
	set(SearchDir "${ARGN}")

	# Retrive all source files recursively
	set(FileExtensions)
	list(APPEND FileExtensions "*.h" "*.hpp" "*.c" "*.cpp" "*.inl")
	if( PLATFORM_OSX OR PLATFORM_IOS )
		list(APPEND FileExtensions "*.m" "*.mm")
    endif()

    if( "${SearchDir}" STREQUAL "" )
		file (GLOB_RECURSE FileList RELATIVE ${PROJECT_SOURCE_DIR} ${FileExtensions})
	else()
		set (UpdatedFileExtensions)
		foreach (FileExtension ${FileExtensions})
			list (APPEND UpdatedFileExtensions "${SearchDir}/${FileExtension}")
		endforeach ()
		file(GLOB_RECURSE FileList RELATIVE ${PROJECT_SOURCE_DIR} ${UpdatedFileExtensions})
	endif()
	list(APPEND ${pResult} ${FileList})

	set_source_groups("${FileList}")
endmacro ()

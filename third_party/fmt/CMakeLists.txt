#
# Copyright (c) 2017-2019 Amer Koleci and contributors.
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

include(CheckSymbolExists)
if (WIN32)
  check_symbol_exists(open io.h HAVE_OPEN)
else ()
  check_symbol_exists(open fcntl.h HAVE_OPEN)
endif ()

# Define the fmt library, its includes and the needed defines.
# *.cc are added to FMT_HEADERS for the header-only configuration.
set(FMT_HEADERS container.h format.h format.cc ostream.h ostream.cc printf.h
                printf.cc string.h time.h)
if (HAVE_OPEN)
  set(FMT_HEADERS ${FMT_HEADERS} fmt/posix.h)
  set(FMT_SOURCES ${FMT_SOURCES} fmt/posix.cc)
endif ()

file (GLOB_RECURSE SOURCE_FILES fmt/*.h)
add_library(fmt STATIC ${SOURCE_FILES} ${FMT_SOURCES} fmt/format.cc)

target_compile_definitions(fmt PUBLIC FMT_USE_RVALUE_REFERENCES=1 FMT_USE_VARIADIC_TEMPLATES=1)
target_include_directories(fmt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${DEST_THIRDPARTY_HEADERS_DIR}>
)

if (ALIMER_SHARED)
    target_compile_definitions(fmt PUBLIC -DFMT_SHARED PRIVATE -DFMT_EXPORT)
endif ()

install (DIRECTORY fmt DESTINATION ${DEST_THIRDPARTY_HEADERS_DIR}/ FILES_MATCHING PATTERN *.h)

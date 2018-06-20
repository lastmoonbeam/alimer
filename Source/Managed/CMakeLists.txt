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

# Native .NET bindings
add_subdirectory(libAlimerSharp)

# Find dotnet program
find_program(DOTNET_BINARY dotnet)
if (NOT DOTNET_BINARY)
    message (FATAL_ERROR "Please make sure dotnet is on the PATH.")
endif ()

# CMake install has issues with new C# project format.
if (NOT DEFINED ENV{APPVEYOR})
    add_target_csharp(AlimerSharp ${CMAKE_CURRENT_SOURCE_DIR}/AlimerSharp/AlimerSharp.csproj)
    set_target_properties(AlimerSharp PROPERTIES FOLDER Managed)

    if (PLATFORM_DESKTOP)
        add_subdirectory(Assets)
        add_subdirectory(Editor)
    endif (PLATFORM_DESKTOP)
endif()
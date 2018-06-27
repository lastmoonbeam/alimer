cmake_minimum_required(VERSION 3.5)

if( _SETTINGS_GUARD )
	return()
endif()
set(_SETTINGS_GUARD 1)

include(CMakePlatforms)
include(CMakeMacros)

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
    execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()
string(REGEX REPLACE "=[^\n]*\n?" ";" ENVIRONMENT "${ENVIRONMENT}")
set(IMPORT_ALIMER_VARIABLES_FROM_ENV BUILD_SHARED_LIBS)
foreach(key ${ENVIRONMENT})
    list (FIND IMPORT_ALIMER_VARIABLES_FROM_ENV ${key} _index)
    if ("${key}" MATCHES "^(ALIMER_|CMAKE_|ANDROID_).+" OR ${_index} GREATER -1)
        if (NOT DEFINED ${key})
            set (${key} $ENV{${key}} CACHE STRING "" FORCE)
        endif ()
    endif ()
endforeach()

# Determine library type
string(TOUPPER "${BUILD_SHARED_LIBS}" BUILD_SHARED_LIBS)
if ("${BUILD_SHARED_LIBS}" STREQUAL "MODULE")
    set (BUILD_SHARED_LIBS OFF)
    set (ALIMER_LIBRARY_TYPE MODULE)
elseif (BUILD_SHARED_LIBS)
    set (ALIMER_LIBRARY_TYPE SHARED)
else ()
    set (ALIMER_LIBRARY_TYPE STATIC)
endif ()


if (PLATFORM_WINDOWS)
        set (ALIMER_STATIC_RUNTIME_DEFAULT ON)
    elseif (PLATFORM_UWP)
        set (ALIMER_STATIC_RUNTIME_DEFAULT OFF)
	else ()
		set (ALIMER_STATIC_RUNTIME_DEFAULT OFF)
endif ()

# Graphics backends
if (NOT ALIMER_DISABLE_VULKAN)
	if (PLATFORM_WINDOWS OR PLATFORM_LINUX OR PLATFORM_ANDROID)
		set (ALIMER_VULKAN_DEFAULT ON)
	else ()
		set (ALIMER_VULKAN_DEFAULT OFF)
	endif ()
endif ()

if (NOT ALIMER_DISABLE_D3D11)
	if (PLATFORM_WINDOWS OR PLATFORM_UWP)
		set (ALIMER_D3D11_DEFAULT ON)
	else ()
		set (ALIMER_D3D11_DEFAULT OFF)
	endif ()
endif ()

if (NOT ALIMER_DISABLE_D3D12)
	if (PLATFORM_WINDOWS OR PLATFORM_UWP)
		set (ALIMER_D3D12_DEFAULT ON)
	else ()
		set (ALIMER_D3D12_DEFAULT OFF)
	endif ()
endif ()

if (NOT ALIMER_DISABLE_SHADER_COMPILER)
	set (ALIMER_SHADER_COMPILER_DEFAULT ON)
endif ()

if (NOT ALIMER_DISABLE_CSHARP)
	if (PLATFORM_DESKTOP)
		set (ALIMER_CSHARP_DEFAULT ON)
	else ()
		set (ALIMER_CSHARP_DEFAULT OFF)
	endif()
endif ()

# Tools
if (NOT ALIMER_DISABLE_ASSET_PIPELINE)
	if (PLATFORM_DESKTOP)
		set (ALIMER_ASSET_PIPELINE_DEFAULT ON)
	else ()
		set (ALIMER_ASSET_PIPELINE_DEFAULT OFF)
	endif()
endif ()

option (ALIMER_STATIC_RUNTIME "Enable link to static runtime" ${ALIMER_STATIC_RUNTIME_DEFAULT})
option (ALIMER_VULKAN "Enable Vulkan backend" ${ALIMER_VULKAN_DEFAULT})
option (ALIMER_D3D11 "Enable D3D11 backend" ${ALIMER_D3D11_DEFAULT})
option (ALIMER_D3D12 "Enable D3D12 backend" ${ALIMER_D3D12_DEFAULT})
option (ALIMER_SHADER_COMPILER "Enable ShaderCompiler" ${ALIMER_SHADER_COMPILER_DEFAULT})
option (ALIMER_CSHARP "Enable C# support" ${ALIMER_CSHARP_DEFAULT})
option (ALIMER_ASSET_PIPELINE "Enable AssetPipeline" ${ALIMER_ASSET_PIPELINE_DEFAULT})

cmake_minimum_required(VERSION 3.20)
project("wxTimeProfilerApp")

# --------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# --------------------------------------------------------------------

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(NDEBUG)
	add_definitions(-DNDEBUG=1)
endif() 

if(NOT(CMAKE_BUILD_TYPE))
	set(CMAKE_BUILD_TYPE "Debug")
	add_definitions(-DDEBUG)
endif()


if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_compile_options("-Wno-comment")
	add_definitions(-DDEBUG_DIR="${CMAKE_SOURCE_DIR}")
endif()

add_compile_options("-O3")

#--------------------------------------------------------------------- 

if(DEFINED "ENV{WXWIDGETS_PATH}")
	find_package(wxWidgets REQUIRED 
		PATHS "$ENV{WXWIDGETS_PATH}" NO_DEFAULT_PATH
		COMPONENTS base core webview
	)
else()
	find_package(wxWidgets COMPONENTS base core webview REQUIRED)
	include("${wxWidgets_INCLUDE_DIRS}")
endif()

message("wxWidgets version: ${wxWidgets_VERSION_STRING}")

# --------------------------------------------------------------------

set(App wxElapsedTimeVisualizer)

add_executable(
	"${App}"
	elapsed_time_visualizer.cpp
)

# link required libs
target_link_libraries(
	"${App}"
	"${wxWidgets_LIBRARIES}"
)

# install resources
add_subdirectory(resources)

#=====================================================================

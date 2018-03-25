# travis has cmake 3.9
cmake_minimum_required(VERSION 3.9)

project(minimame)

enable_testing()

set(PROJECT_NAME "Mini Mame")
set(PROJECT_Version 0.1)
set(PROJECT_LIBS)

set(COVERAGE OFF CACHE BOOL "Coverage")

find_package(Boost)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MP")
endif()
if (COVERAGE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release"
		FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-D_UNICODE -DUNICODE)
endif()

set(CORE_SRC_FILES
  source/core/attotime.cpp
  source/core/attotime.h
  source/core/delegate.cpp
  source/core/delegate.h
)

set(EMUCORE_SRC_FILES
  source/emucore/devdelegate.cpp
  source/emucore/devdelegate.h
  source/emucore/memory/dimemory.cpp
)

set(SRC_FILES
  source/sdlmain.cpp
)

add_library(core STATIC ${CORE_SRC_FILES})
add_library(emucore STATIC ${EMUCORE_SRC_FILES})

add_executable(minimame ${SRC_FILES})
target_link_libraries(minimame emucore core)

include(BoostTestHelpers.cmake)
add_boost_test(tests/emu/attotime.cpp core)

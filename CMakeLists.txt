# CMake File for MojoShader Branch
# Written by Ethan "flibitijibibo" Lee

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(mojoshader)

# Build Options
OPTION(MS_DEBUG "Build MojoShader with debugging symbols" ON)

# Compiler Flags
IF(MS_DEBUG)
	SET(CMAKE_CXX_FLAGS "-g -Wall -pedantic")
ELSE()
	SET(CMAKE_CXX_FLAGS "-O2 -Wall -pedantic")
ENDIF()

# Linker Flags
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Defines
ADD_DEFINITIONS(-DMOJOSHADER_NO_VERSION_INCLUDE)

# Source Lists
SET(MOJOSHADER_SRC
	mojoshader.c
	mojoshader_common.c
	mojoshader_opengl.c
	mojoshader_effects.c
)

# Targets
ADD_LIBRARY(mojoshader SHARED ${MOJOSHADER_SRC})
TARGET_LINK_LIBRARIES(mojoshader m)

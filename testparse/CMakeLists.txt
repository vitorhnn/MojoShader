CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(testparse)

set(TESTPARSE_SRC
        testparse.c
)

add_executable(${PROJECT_NAME} ${TESTPARSE_SRC})

add_library(spirv-tools SHARED IMPORTED)
set_property(TARGET spirv-tools PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/SPIRV-Tools-shared.dll)
set_property(TARGET spirv-tools PROPERTY IMPORTED_IMPLIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/SPIRV-Tools-shared.lib)

# Implicitly adds mojoshader as a dependency
target_link_libraries(${PROJECT_NAME}
        PRIVATE mojoshader
        PRIVATE spirv-tools)

include_directories(include/)

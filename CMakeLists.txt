cmake_minimum_required(VERSION 3.28)

set(WHICHWAD_PROJECT_NAME         "whichwad")
set(WHICHWAD_PROJECT_DESCRIPTION  "Find which WAD files contains the specificed textures")
set(WHICHWAD_AUTHOR_DOMAIN        "https://github.com/erty-gamedev/whichwad")
set(WHICHWAD_AUTHOR_MAINTAINER    "erty.gamedev@gmail.com")
set(WHICHWAD_VERSION_MAJOR        "3")
set(WHICHWAD_VERSION_MINOR        "1")
set(WHICHWAD_VERSION_PATCH        "0")
set(WHICHWAD_VERSION              "${WHICHWAD_VERSION_MAJOR}.${WHICHWAD_VERSION_MINOR}.${WHICHWAD_VERSION_PATCH}")
set(WHICHWAD_NAME_VERSION         "Which WAD v${WHICHWAD_VERSION}")

add_compile_definitions(WHICHWAD_NAME_VERSION="${WHICHWAD_NAME_VERSION}")

project(${WHICHWAD_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(PUBLIC "${PROJECT_SOURCE_DIR}/vendor/logging/src")

add_executable(${WHICHWAD_PROJECT_NAME}
    src/main.cpp
    src/utils.cpp
    src/whichwad.cpp
    src/bmp8bpp.cpp
)

set(WHICHWAD_BIN_DIR "${PROJECT_SOURCE_DIR}/bin")

set_target_properties(${WHICHWAD_PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${WHICHWAD_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${WHICHWAD_BIN_DIR}/debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${WHICHWAD_BIN_DIR}/release"
)

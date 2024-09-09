cmake_minimum_required(VERSION 3.3)

set(WHICHWAD_PROJECT_NAME         "whichwad")
set(WHICHWAD_PROJECT_DESCRIPTION  "Find which WAD files contains specific textures")
set(WHICHWAD_AUTHOR_DOMAIN        "https://github.com/erty-gamedev/whichwad")
set(WHICHWAD_AUTHOR_MAINTAINER    "erty.gamedev@gmail.com")
set(WHICHWAD_VERSION_MAJOR        "1")
set(WHICHWAD_VERSION_MINOR        "0")
set(WHICHWAD_VERSION_PATCH        "0")
set(WHICHWAD_VERSION              "${WHICHWAD_VERSION_MAJOR}.${WHICHWAD_VERSION_MINOR}.${WHICHWAD_VERSION_PATCH}")
set(WHICHWAD_NAME_VERSION         "Which WAD v${WHICHWAD_VERSION}")

project(${WHICHWAD_PROJECT_NAME})

add_executable(${WHICHWAD_PROJECT_NAME}
    whichwad.cpp
    utils.cpp
    wad3reader.cpp
    consolestyling.cpp
)

configure_file(utils.h.in utils.h @ONLY)

target_include_directories(${WHICHWAD_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
target_compile_features(${WHICHWAD_PROJECT_NAME} PUBLIC cxx_std_17)

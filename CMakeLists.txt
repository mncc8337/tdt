cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /fp:fast /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -flto")
endif()

include(FetchContent)

FetchContent_Declare(sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_Declare(imgui
    URL "https://github.com/ocornut/imgui/archive/v1.91.9b.zip"
)

FetchContent_Declare(imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

option(SFML_BUILD_AUDIO "Build audio" OFF)
option(SFML_BUILD_NETWORK "Build network" OFF)

FetchContent_MakeAvailable(sfml)
FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)

FetchContent_MakeAvailable(imgui-sfml)

find_package(OpenMP REQUIRED)

add_executable(tdt)

target_compile_features(tdt PRIVATE cxx_std_20)

target_link_libraries(tdt
    PRIVATE
      SFML::Graphics
      OpenMP::OpenMP_CXX
    PUBLIC
      ImGui-SFML::ImGui-SFML
)

target_include_directories(tdt PRIVATE src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tdt)

add_subdirectory(src)

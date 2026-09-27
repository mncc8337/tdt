cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(SFML
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

add_executable(
    main
    src/main.cpp
    src/Color.cpp
    src/Vec3.cpp
    src/RayTracer.cpp
    src/Ray.cpp
    src/Camera.cpp
    src/HitInfo.cpp
    src/Hittable.cpp
    src/Sphere.cpp
    src/Triangle.cpp
)

target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(
    main
    PRIVATE
        SFML::Graphics
    PUBLIC
        ImGui-SFML::ImGui-SFML
)

target_include_directories(main PRIVATE include)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

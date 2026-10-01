target_sources(tdt
  PRIVATE 
    Scene.cpp
    Camera.cpp
    HitInfo.cpp
    Hittable.cpp
    RayTracer.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(hittable)

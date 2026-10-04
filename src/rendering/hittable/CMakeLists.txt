target_sources(tdt
  PRIVATE 
    Sphere.cpp
    Triangle.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

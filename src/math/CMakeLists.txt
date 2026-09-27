target_sources(tdt
  PRIVATE
    Color.cpp
    Ray.cpp
    Vec3.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

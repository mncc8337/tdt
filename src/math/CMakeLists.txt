target_sources(tdt
  PRIVATE
    Vec3.cpp
    Vec2.cpp
    Ray.cpp
    AABB.cpp
    RawTriangle.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

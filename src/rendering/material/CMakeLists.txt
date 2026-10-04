target_sources(tdt
  PRIVATE 
    Metal.cpp
    Matte.cpp
    Dielectric.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

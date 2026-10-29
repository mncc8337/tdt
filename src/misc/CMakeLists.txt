target_sources(tdt
  PRIVATE 
    RNG.cpp
    meshloader.cpp
    ColorUtils.cpp
    NamedElement.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(tdt
  PRIVATE 
    SolidTexture.cpp
    ImageTexture.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

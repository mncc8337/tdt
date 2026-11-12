target_sources(tdt
  PRIVATE
    Gui.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# nativefiledialog-extended provides a proper CMake target (and its
# include directories propagate from it), so just link it.
target_link_libraries(tdt PRIVATE nfd)

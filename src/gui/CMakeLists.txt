target_sources(tdt
  PRIVATE
    Gui.cpp
)

target_include_directories(tdt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# build nfd since it has no CMakeList.txt
set(NFD_DIR ${nativefiledialog_SOURCE_DIR})

if(WIN32)
    set(NFD_BACKEND ${NFD_DIR}/src/nfd_win.cpp)
elseif(APPLE)
    enable_language(OBJC)
    set(NFD_BACKEND ${NFD_DIR}/src/nfd_cocoa.m)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
    set(NFD_BACKEND ${NFD_DIR}/src/nfd_gtk.c)
    set(NFD_LIBS PkgConfig::GTK3)
endif()

target_sources(tdt PRIVATE
    ${NFD_DIR}/src/nfd_common.c
    ${NFD_BACKEND}
)

target_include_directories(tdt PRIVATE ${NFD_DIR}/src/include)

if(NFD_LIBS)
    target_link_libraries(tdt PRIVATE ${NFD_LIBS})
endif()

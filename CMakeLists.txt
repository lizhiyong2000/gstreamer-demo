cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 ")


#set(ENV{PKG_CONFIG_PATH} "/Library/Frameworks/GStreamer.framework/Libraries/pkgconfig")

find_package(PkgConfig REQUIRED)


pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

pkg_check_modules(GLIB REQUIRED glib-2.0)


pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET opencv)

include_directories(
  ${GLIB_INCLUDE_DIRS}
)

link_directories(
  ${GLIB_LIBRARY_DIRS}
)




include_directories(
   ${GSTREAMER_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

link_directories(
  ${GSTREAMER_LIBRARY_DIRS}
)

set(gst_SOURCES
	
    main.cpp
)


set(gst_LIBRARIES
  ${GSTREAMER_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GLIB_GOBJECT_LIBRARIES}

  ${GSTREAMER}
)

add_executable(gst_test  ${gst_SOURCES})
target_link_libraries(gst_test ${gst_LIBRARIES})


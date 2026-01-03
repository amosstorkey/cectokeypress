project(ceckeypress)
cmake_minimum_required(VERSION 3.12.0)

set(LIBCEC_VERSION_MAJOR 6)
set(LIBCEC_VERSION_MINOR 1)
set(LIBCEC_VERSION_PATCH 1)

set(ceckeypress_NAME ceckeypress)
set(ceckeypress_DESCRIPTION "CEC remote to keyboar converter")
set(ceckeypress_VERSION_MAJOR 1)
set(ceckeypress_VERSION_MINOR 0)
set(ceckeypress_VERSION_PATCH 0)

enable_language(CXX)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

check_cxx_compiler_flag("-std=c++11" SUPPORTS_CXX11)
if (SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(Threads REQUIRED)

set(ceckeypress_SOURCES ceckeypress.cpp)

# curses
check_library_exists(curses initscr "" HAVE_CURSES_API)
if (HAVE_CURSES_API)
  list(APPEND ceckeypress_SOURCES curses/CursesControl.cpp)

  # tinfo
  find_library(HAVE_CURSES_TINFO tinfo)
endif()


add_executable(ceckeypress ${ceckeypress_SOURCES})
set_target_properties(ceckeypress PROPERTIES VERSION 1.0.0)
target_link_libraries(ceckeypress ${CMAKE_THREAD_LIBS_INIT})

if (NOT WIN32)
  # check for dlopen
  check_library_exists(dl dlopen "" HAVE_DLOPEN)
  if (HAVE_DLOPEN)
    target_link_libraries(ceckeypress dl)
  endif()

  # curses
  if (HAVE_CURSES_API)
    target_link_libraries(ceckeypress curses)
    if (HAVE_CURSES_TINFO)
      target_link_libraries(ceckeypress tinfo)
    endif()
  endif()

  # rt
  check_library_exists(rt clock_gettime "" HAVE_RT)
  if (HAVE_RT)
    target_link_libraries(ceckeypress rt)
  endif()

  # CoreVideo
  if (APPLE)
    target_link_libraries(ceckeypress "-framework CoreVideo")
  endif()
else()
  add_definitions(-DTARGET_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCKAPI_)
  check_symbol_exists(_AMD64_ Windows.h WIN64)
  check_symbol_exists(_ARM64_ Windows.h ARM64)
  if (WIN64 OR ARM64)
    string(REPLACE "/arch:SSE2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  else()
    add_definitions(-D_USE_32BIT_TIME_T)
  endif()
endif()

include_directories(
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/include)

# version number
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h)

# write env.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/env.h.in ${CMAKE_CURRENT_SOURCE_DIR}/env.h)

if (WIN32)
  install(TARGETS     ceckeypress
          DESTINATION .)
else()
  install(TARGETS     ceckeypress
          DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

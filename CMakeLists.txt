cmake_minimum_required(VERSION 3.16)
project(MuoneEventDisplay LANGUAGES CXX)

# ---- Locate ROOT automatically (no user variables needed) ----
set(_root_hints "")

# 1. root-config on PATH (works after sourcing thisroot.sh)
find_program(ROOT_CONFIG_EXECUTABLE root-config
  HINTS $ENV{ROOTSYS}/bin $ENV{CONDA_PREFIX}/bin)
if(ROOT_CONFIG_EXECUTABLE)
  execute_process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
    OUTPUT_VARIABLE _root_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
  list(APPEND _root_hints ${_root_prefix})
endif()

# 2. Environment variables
foreach(_v ROOTSYS CONDA_PREFIX FAIRROOTPATH SIMPATH)
  if(DEFINED ENV{${_v}})
    list(APPEND _root_hints $ENV{${_v}})
  endif()
endforeach()

# 3. Common install locations
list(APPEND _root_hints /usr /usr/local /opt/root /opt/homebrew /snap/root-framework/current)

find_package(ROOT CONFIG QUIET
  COMPONENTS Core RIO Tree TreePlayer
  HINTS ${_root_hints}
  PATH_SUFFIXES cmake share/root/cmake lib/cmake/ROOT lib/root/cmake)

if(NOT ROOT_FOUND)
  message(FATAL_ERROR
    "ROOT was not found.\n"
    "Install it with one of:\n"
    "  sudo apt install root-system   (Ubuntu/Debian)\n"
    "  brew install root              (macOS)\n"
    "or run 'source <root>/bin/thisroot.sh' and re-run cmake.")
endif()
message(STATUS "Using ROOT ${ROOT_VERSION} from ${ROOT_DIR}")

# ---- Build settings ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# executables end up in <build>/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- export_event: ROOT file -> JSON, used by the web event display (see scripts/ and web/) ----
add_executable(export_event
  tools/export_event/main.cpp
  tools/export_event/event_reader.cpp
  tools/export_event/json_writer.cpp)
target_include_directories(export_event PRIVATE tools/export_event)
target_link_libraries(export_event PRIVATE ROOT::Core ROOT::RIO ROOT::Tree ROOT::TreePlayer)
target_compile_options(export_event PRIVATE -Wall -Wextra -Wpedantic)

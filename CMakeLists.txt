cmake_minimum_required(VERSION 3.16)
project(TrackAnalysis LANGUAGES CXX)

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
  COMPONENTS Core RIO Tree TreePlayer Hist Gpad Graf Graf3d MathCore Imt
  HINTS ${_root_hints}
  PATH_SUFFIXES cmake share/root/cmake lib/cmake/ROOT lib/root/cmake)

if(NOT ROOT_FOUND)
  message(FATAL_ERROR
    "ROOT was not found.\n"
    "Install it with one of:\n"
    "  conda env create -f environment.yml && conda activate track-analysis\n"
    "  sudo apt install root-system   (Ubuntu/Debian)\n"
    "  brew install root              (macOS)\n"
    "or run 'source <root>/bin/thisroot.sh' and re-run cmake.")
endif()
message(STATUS "Using ROOT ${ROOT_VERSION} from ${ROOT_DIR}")

# ---- Rest of the project as before ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(ROOT_LIBS
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::TreePlayer
  ROOT::Hist ROOT::Gpad ROOT::Graf ROOT::Graf3d ROOT::MathCore ROOT::Imt)

# everything except main.cpp goes into a library
file(GLOB LIB_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

add_library(trackcore STATIC ${LIB_SOURCES})
target_include_directories(trackcore PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(trackcore PUBLIC ${ROOT_LIBS})

add_executable(TrackAnalysis src/main.cpp)
target_link_libraries(TrackAnalysis PRIVATE trackcore)

# ---- tests ----
include(CTest)
if(BUILD_TESTING)
  add_executable(analyze_tests tests/test_analyze.cpp)
  target_link_libraries(analyze_tests PRIVATE trackcore)
  add_test(NAME analyze_tests COMMAND analyze_tests)
endif()
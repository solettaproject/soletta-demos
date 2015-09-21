cmake_minimum_required(VERSION 2.8)

project(soletta-demos)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-Wall" CXX_COMPILER_HAS_WALL)
CHECK_CXX_COMPILER_FLAG("-Wextra" CXX_COMPILER_WALL_HAS_WEXTRA)
CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" CXX_COMPILER_HAS_FVISIBILITY)

CHECK_C_COMPILER_FLAG("-std=gnu99" C_COMPILER_HAS_C99)
CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_HAS_WALL)
CHECK_C_COMPILER_FLAG("-Wextra" C_COMPILER_HAS_WEXTRA)
CHECK_C_COMPILER_FLAG("-Wno-unused-parameter" C_COMPILER_HAS_WNO_UNUSED_PARAMETER)
CHECK_C_COMPILER_FLAG("-Wno-missing-field-initializers" C_COMPILER_HAS_WNO_MISSING_FIELD_INITIALIZERS)
CHECK_C_COMPILER_FLAG("-fvisibility=hidden" C_COMPILER_HAS_FVISIBILITY)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")

if (CXX_COMPILER_HAS_WALL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if (CXX_COMPILER_WALL_HAS_WEXTRA)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

if (CXX_COMPILER_HAS_FVISIBILITY)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()

if (C_COMPILER_HAS_FVISIBILITY)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

if (C_COMPILER_HAS_WEXTRA)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
endif()

if (C_COMPILER_HAS_WALL)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

if (C_COMPILER_HAS_C99)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
else()
  message(FATAL_ERROR "No gnu99 support. Bye.")
endif()

if (C_COMPILER_HAS_WNO_UNUSED_PARAMETER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
endif()

if (C_COMPILER_HAS_WNO_MISSING_FIELD_INITIALIZERS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SOLETTA REQUIRED soletta)

add_subdirectory(foosball)

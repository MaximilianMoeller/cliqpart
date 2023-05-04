# This CMake-File is heavily inspired by
#   - this article in the gurobi forum: https://support.gurobi.com/hc/en-us/articles/360039499751-How-do-I-use-CMake-to-build-Gurobi-C-C-projects
#   - Radovan Bast‘s “CMake Cookbook”, Chapter “Structuring Projects”

cmake_minimum_required(VERSION 3.16)
project(cliqpart
        VERSION 0.1
        DESCRIPTION "Implementation of a branch and cut algorithm for the clique partitioning problem."
        LANGUAGES CXX)

# Includes all CMake-Files in the cmake directory.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Default build type is “Release”
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Enable C++ compilation unless explicitly deselected.
# NOTE: This is no longer needed, as CXX is selected as the default language for this project.
# NOTE: However, all occurrences of “if(CXX) …” will remain such as to keep the original code from the gurobi forum.
# option(CXX "enable C++ compilation" ON)
if(CXX)
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Set output directories for all artefacts that are built under UNIX.
if(UNIX)
    # Helpers such as ${CMAKE_INSTALL_<dir>}
    include(GNUInstallDirs)
    # Place created binaries under BUILD-DIR/bin …
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
            ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
    # … and everything else under BUILD-DIR/lib
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
        ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()

# Some Windows stuff I don't understand
if(MSVC AND MT)
    # Visual Studio compiler with static runtime libraries
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})

# All source code is under src/
add_subdirectory(src)

# Only execute if this file is the toplevel being build (i.e. not a sub-goal in another CMake-File)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    # Prints summary information about enabled and disabled packages (e.g. Gurobi).
    include(FeatureSummary)
    feature_summary(WHAT ALL)
endif()

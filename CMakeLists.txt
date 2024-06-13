# This CMake-File is heavily inspired by
#   - this article in the gurobi forum: https://support.gurobi.com/hc/en-us/articles/360039499751-How-do-I-use-CMake-to-build-Gurobi-C-C-projects
#   - Radovan Bast‘s “CMake Cookbook”, Chapter “Structuring Projects”

cmake_minimum_required(VERSION 3.16)
project(cliqpart
        VERSION 0.1
        DESCRIPTION "Implementation of a branch and cut algorithm for the clique partitioning problem."
        LANGUAGES CXX)

# creates a `compile_commands.json`, which is used by further clang tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# symlink the createy compile_commands.json
if (PROJECT_IS_TOP_LEVEL AND UNIX)
    execute_process(
		# ${CMAKE_COMMAND} is just the `cmake`-binary
		# -E makes `cmake` run a command-line tool
		# `create_symlink` is such a command-line tool
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

option(CXX "enable C++ compilation" ON)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type is “Release”
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")


# Includes all CMake-Files in the cmake directory.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


# Enable C++ compilation unless explicitly deselected.
# NOTE: This is no longer needed, as CXX is selected as the default language for this project.
# NOTE: However, all occurrences of “if(CXX) …” will remain such as to keep the original code from the gurobi forum.

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

# Define the target name, i.e. “cliqpart”
add_executable(${CMAKE_PROJECT_NAME})

# Add source code
add_subdirectory(src)

### Adding Libraries ###
# gurobi for ILP-solving
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})

# CLI11 for command line parsing
set(CLI11_TESTING OFF)
set(CLI11_PRECOMPILED ON)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "extern/CLI11/include")

# plog for logging
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "extern/plog/include")

# rapidcsv for CSV file parsing
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "extern/rapidcsv/src")

# tomplplusplus for config file parsing
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "extern/tomlplusplus/include")

# progress bar
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "extern/progressbar/include")

# Link Libraries
if(CXX)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE optimized ${GUROBI_CXX_LIBRARY}
            debug ${GUROBI_CXX_DEBUG_LIBRARY})
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${GUROBI_LIBRARY})

# Only execute if this file is the toplevel being build (i.e. not a sub-goal in another CMake-File)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    # Prints summary information about enabled and disabled packages (e.g. Gurobi).
    include(FeatureSummary)
    feature_summary(WHAT ALL)
endif()

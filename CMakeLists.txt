cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(RemoveEffect VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

# Use a working bindings commit (HEAD~1 fixes Broma parse error: /// inside class body)
set(BINDINGS_COMMIT "31bd965a")
set(BINDINGS_CLONE_PATH "${CMAKE_CURRENT_BINARY_DIR}/bindings-src")
if (NOT EXISTS "${BINDINGS_CLONE_PATH}/.git")
    file(MAKE_DIRECTORY "${BINDINGS_CLONE_PATH}")
    execute_process(
        COMMAND git init
        WORKING_DIRECTORY "${BINDINGS_CLONE_PATH}"
        OUTPUT_QUIET ERROR_QUIET
    )
    execute_process(
        COMMAND git fetch --depth=1 https://github.com/geode-sdk/bindings.git ${BINDINGS_COMMIT}
        WORKING_DIRECTORY "${BINDINGS_CLONE_PATH}"
        OUTPUT_QUIET ERROR_QUIET
    )
    execute_process(
        COMMAND git checkout FETCH_HEAD
        WORKING_DIRECTORY "${BINDINGS_CLONE_PATH}"
        OUTPUT_QUIET ERROR_QUIET
    )
endif()
if (EXISTS "${BINDINGS_CLONE_PATH}/bindings/2.2081/GeometryDash.bro")
    set(ENV{GEODE_BINDINGS_REPO_PATH} "${BINDINGS_CLONE_PATH}")
    set(GEODE_BINDINGS_REPO_PATH "${BINDINGS_CLONE_PATH}" CACHE PATH "Working bindings" FORCE)
    message(STATUS "Using pinned bindings commit ${BINDINGS_COMMIT}")
else()
    message(WARNING "Failed to clone bindings, will use default (may fail)")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})

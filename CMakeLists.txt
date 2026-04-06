cmake_minimum_required(VERSION 3.20)

project(IAGHI)
enable_testing()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IAGHI_IS_TOP_LEVEL ON)
else()
    set(IAGHI_IS_TOP_LEVEL OFF)
endif()

set(IAGHI_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

if(TARGET libauxid)
    message(STATUS "IAGHI: Using external LibAuxid provided by parent project")
else()
    message(STATUS "IAGHI: Building standalone, using local LibAuxid submodule")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libauxid/CMakeLists.txt")
        message(FATAL_ERROR "LibAuxid submodule missing! Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(libauxid)
endif()

auxid_setup_project()

option(IAGHI_BUILD_TESTS "Build unit tests" ${IAGHI_IS_TOP_LEVEL})
option(IAGHI_BUILD_SANDBOX "Build sandbox project" ${IAGHI_IS_TOP_LEVEL})

include(cmake/find_deps.cmake)

add_subdirectory(src)

if(IAGHI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(IAGHI_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif ()

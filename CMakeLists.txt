cmake_minimum_required(VERSION 3.5)


# ==============================================================================
# Project info

# Name
project(SCIP-example)


# ==============================================================================
# C++ standard

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)


# ==============================================================================
# Path to external dependencies

# SCIP headers
if (NOT DEFINED SCIP_HEADERS_PATH)
    set(SCIP_HEADERS_PATH "/opt/scip/include")
endif()

# SCIP libraries
if (NOT DEFINED SCIP_LIB_PATH)
    set(SCIP_LIB_PATH "/opt/scip/lib")
endif()


# ==============================================================================
# Paths to search for headers

include_directories(
        ${PROJECT_SOURCE_DIR}/lib/
        ${SCIP_HEADERS_PATH}/)


# ==============================================================================
# Paths to search for libraries

link_directories(
        ${SCIP_LIB_PATH}/)


# ==============================================================================
# Libraries to link

set(SCIP_LIBS  scip soplex)
set(OTHER_LIBS pthread m z)


# ==============================================================================
# Source files

set(SOURCE_FILES main.cpp)


# ==============================================================================
# Targets

add_executable(scip_example ${SOURCE_FILES})
target_link_libraries(scip_example ${SCIP_LIBS} ${OTHER_LIBS})

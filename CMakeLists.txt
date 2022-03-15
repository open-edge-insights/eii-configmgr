# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.15)
project(eii-configmanager
    VERSION 3.0.0
    LANGUAGES C CXX
    DESCRIPTION "Intel's Edge Insights for Industrial (EII) C/C++ Configuration Manager Library"
    HOMEPAGE_URL "https://github.com/open-edge-insights/eii-core")

set(PROJECT_LICENSE "MIT")

# Define CMake options
option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_GO       "Compile Go Bindings" OFF)
option(WITH_PYTHON   "Compile with Python bindings" OFF)
option(WITH_TESTS    "Compile with tests" OFF)
option(SYSTEM_GRPC   "Use the system installed gRPC" OFF)
option(WITH_DOCS     "Generate ConfigMgr documentation" OFF)


# Verify that packaging is off if SYSTEM_GRPC is turned on
# This is not supported due to complications with linking, the target system the
# package would be installed on cannot be assumed to be structured the same as
# the system the library is built on.
if(PACKAGING AND SYSTEM_GRPC)
    message(FATAL_ERROR "Packaging cannot be done when using the system's gRPC version")
endif()


# Globals
set(EII_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

# Set C standard to C11
set(CMAKE_CXX_STANDARD 11)

# Set CFLAGS
set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Set LDFLAGS
set(CMAKE_SHARED_LINKER_FLAGS "-z noexecstack -z relro -z now")

# Add cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${EII_COMMON_CMAKE}")

# Finding libraries
find_package(Threads REQUIRED)
find_package(EIIUtils REQUIRED)
find_package(EIIMsgEnv REQUIRED)
find_package(IntelSafeString REQUIRED)

# Install Go EnvConfig if specified to
if(WITH_GO)
    message("-- Configuring Go ConfigMgr installation")

    # Verify the $GOPATH environmental variable exists
    if("$ENV{GOPATH}" STREQUAL "")
        message(FATAL_ERROR "-- $GOPATH envronmental variable is not set")
    endif()

    # Configure install command to copy the Go ConfigMgr to the
    # user's $GOPATH
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/go/ConfigMgr
        DESTINATION
            $ENV{GOPATH}/src/
    )
endif()

# Include header directories
include_directories(
    ${CMAKE_INSTALL_PREFIX}/include/
    include/
    ${EIIMsgEnv_INCLUDE}
    ${EIIUtils_INCLUDE}
    ${IntelSafeString_INCLUDE})

link_directories(${CMAKE_INSTALL_PREFIX}/lib)

# Get all source files
file(GLOB SOURCES "src/*.c" "cpp/*.cpp" "src/*/*.c" "src/*/etcd_client/*.c" "src/*/etcd_client/*.cpp" "src/*/etcd_client/*/*.cpp")
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

add_library(eiiconfigmanager_static STATIC ${SOURCES})
add_library(eiiconfigmanager SHARED ${SOURCES})

# Link gRPC to the library
include(cmake/LinkGRPC.cmake)

target_link_libraries(eiiconfigmanager
    PUBLIC
        pthread
    PRIVATE
        cjson
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
        ${IntelSafeString_LIBRARIES})

target_link_libraries(eiiconfigmanager_static
    PUBLIC
        pthread
    PRIVATE
        cjson
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
        ${IntelSafeString_LIBRARIES})

# If compile in debug mode, set DEBUG flag for C code
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(eiiconfigmanager PRIVATE DEBUG=1)
endif()

if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()

if(WITH_PYTHON)
    add_subdirectory(python/)
endif()

if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

##
## Documentation generation
##
if(WITH_DOCS)
    message("-- Configuring documentation generation")

    set(DOCS_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")
    set(PDFS_DIR "${DOCS_DIR}/pdfs")

    # Create doxygen directory
    file(MAKE_DIRECTORY ${DOCS_DIR})
    file(MAKE_DIRECTORY ${PDFS_DIR})

    # Doxygen documentation generation
    message("       Configuring C/C++ API documentation")
    find_package(Doxygen REQUIRED)
    find_program(PDFLATEX NAMES pdflatex)

    # Make C documentation directory
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs/c")

    set(DOXYFILE_IN  "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
    set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    # Add target for generating doxygen documentation
    add_custom_target(doxygen-docs
        COMMENT "Generating C/C++ API Doxygen documentation"
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)

    # Add target for generating PDF of C/C++ doxygen documentation
    add_custom_target(doxygen-docs-pdf
        COMMENT "Generating C/C++ API PDF"
        DEPENDS doxygen-docs
        COMMAND ${CMAKE_COMMAND} -E remove "${PDFS_DIR}/EII_ConfigMgr_C_Reference_${PROJECT_VERSION}.pdf"
        COMMAND ${PDFLATEX}
            -interaction=nonstopmode
            -output-directory ${PDFS_DIR}
            -jobname=EII_ConfigMgr_C_Reference_${PROJECT_VERSION}
            refman.tex
        WORKING_DIRECTORY ${DOCS_DIR}/c/latex/
        VERBATIM)

    # List of docs dependency targets
    set(DOCS_TARGETS doxygen-docs doxygen-docs-pdf)

    # Final target for compiling all EII ConfigMgr documentation with make docs
    add_custom_target(docs
        COMMENT "Generating EII ConfigMgr documentation"
        DEPENDS ${DOCS_TARGETS}
        VERBATIM)
endif()

# Include CMake script for defining the installation targets of
# the config manager
include(cmake/InstallEIIConfigMgr.cmake)

# Generate DEB package for EIIConfigMgr
set(CPACK_PACKAGE_VENDOR "Intel Corporation")

set(CPACK_PACKAGE_DESCRIPTION "Intel's Edge Insights for Industrial (EII) C/C++ Configuration Manager Library")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/open-edge-insights/eii-core")

# Debian package dependencies
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Intel Corporation")
## TODO: Come up with right way for adding dependency versions
set(CPACK_DEBIAN_PACKAGE_DEPENDS "eii-utils (>= 2.6.0), eii-messagebus (>=2.6.0)")

# RPM package specific settings
set(CPACK_RPM_PACKAGE_REQUIRES "cjson >= 1.7.14, eii-utils >= 2.6.0, eii-messagebus >= 2.6.0, zlib >= 1.2.11")

# APK specific settings
set(APKBUILD_DEPENDS "zeromq eii-utils eii-messagebus cjson zlib")
set(APKBUILD_DEPENDS_DEV "")
set(APKBUILD_DEPENDS_MAKE "zeromq-dev eii-utils eii-messagebus cjson-dev libtool autoconf pkgconf linux-headers zlib-dev")
set(APKBUILD_REQUIRE_EXTERNAL_APKS TRUE)
set(APKBUILD_EXTERNAL_APKS "eii-utils,eii-messagebus")

# Include packaging utility
include(cmake/Package.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json)
endif()

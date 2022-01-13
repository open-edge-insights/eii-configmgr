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

##
## Helper CMake file to link gRPC based on the system configuration
##

if(SYSTEM_GRPC)
    # Find the gRPC package on the system
    find_package(PROTOBUF REQUIRED)
    find_package(GRPC REQUIRED)

    # Add include directories
    include_directories(
        ${GRPC_INCLUDE_DIRS}
        ${PROTOBUF_INCLUDE_DIRS})

    target_link_libraries(eiiconfigmanager_static
        PUBLIC
            ${GRPC_LIBRARIES}
            ${PROTOBUF_LIBRARIES})
    target_link_libraries(eiiconfigmanager
        PUBLIC
            ${GRPC_LIBRARIES}
            ${PROTOBUF_LIBRARIES})
else()
    # Set gRPC_INSTALL to ON so that the gRPC targets get installed with the
    # EII Config Manager targets.
    set(gRPC_INSTALL ON CACHE BOOL "" FORCE)
    set(gRPC_ZLIB_PROVIDER "package" CACHE STRING "" FORCE)

    # Fetch GRPC dependency
    include(FetchContent)
    set(GRPC_VERSION "v1.29.0")
    FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        ${GRPC_VERSION}
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(gRPC)

    target_link_libraries(eiiconfigmanager_static PRIVATE grpc++)
    target_link_libraries(eiiconfigmanager PRIVATE grpc++)
endif()

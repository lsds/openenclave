# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

oeedl_file(../bigmalloc.edl host gen)

add_executable(bigmalloc_host host.c ${gen})

target_include_directories(bigmalloc_host PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bigmalloc_host oehostapp)

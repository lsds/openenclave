# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

oeedl_file(../blob.edl host gen)

add_executable(blob_host host.c ${gen})

target_include_directories(blob_host PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(blob_host oehostapp)

# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

oeedl_file(../socket_test.edl 
    enclave socket_test_t
    --edl-search-dir ../../../include
)

add_enclave(TARGET socket_enc SOURCES enc.c ${socket_test_t})

target_include_directories(socket_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(socket_enc oelibc oehostsock)

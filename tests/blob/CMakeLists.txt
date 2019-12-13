# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_subdirectory(host)
add_subdirectory(enc)

set(BLOB ${CMAKE_CURRENT_SOURCE_DIR}/enc/blob.txt)

add_test(tests/blob
    ${CMAKE_CURRENT_BINARY_DIR}/host/blob_host
    ${CMAKE_CURRENT_BINARY_DIR}/enc/blob_enc_with_blob
    ${BLOB})

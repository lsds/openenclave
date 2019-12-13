# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

##==============================================================================
##
## Rules to create blob_enc and blob_with_blob
##
##==============================================================================

oeedl_file(../blob.edl enclave gen)

add_enclave(TARGET blob_enc
    UUID a72d9831-4c4b-461d-bf83-e4d3d58cc40b SOURCES enc.c ${gen})

target_include_directories(blob_enc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(blob_enc oelibc)

##==============================================================================
##
## Rules to create the enclave with the .oeblob section
##
##==============================================================================

set(ENCLAVE_IN ${CMAKE_CURRENT_BINARY_DIR}/blob_enc)
set(ENCLAVE_OUT ${ENCLAVE_IN}_with_blob)
set(BLOB ${CMAKE_CURRENT_SOURCE_DIR}/blob.txt)

add_custom_command(
    OUTPUT ${ENCLAVE_OUT}
    DEPENDS ${ENCLAVE_IN}
    COMMAND objcopy
        --add-section .oeblob=${BLOB}
        --set-section-flags .oeblob=noload,readonly
        ${ENCLAVE_IN} ${ENCLAVE_OUT})

add_custom_target(add_blob ALL DEPENDS ${ENCLAVE_OUT})

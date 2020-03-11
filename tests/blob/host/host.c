// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <openenclave/internal/files.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blob_u.h"

int main(int argc, const char* argv[])
{
    const char* arg0 = argv[0];
    oe_result_t result;
    oe_enclave_t* enclave;
    const uint32_t flags = oe_get_create_flags();
    const oe_enclave_type_t type = OE_ENCLAVE_TYPE_SGX;
    void* data;
    size_t size;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH BLOB_FILE\n", arg0);
        return 1;
    }

    result = oe_create_blob_enclave(argv[1], type, flags, NULL, 0, &enclave);
    OE_TEST(result == OE_OK);

    result = __oe_load_file(argv[2], 0, &data, &size);
    OE_TEST(result == OE_OK);

    result = test_blob(enclave, data, size);
    OE_TEST(result == OE_OK);

    result = oe_terminate_enclave(enclave);
    OE_TEST(result == OE_OK);

    free(data);

    printf("=== passed all tests (host)\n");

    return 0;
}

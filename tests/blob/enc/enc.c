// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/globals.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <string.h>
#include "blob_t.h"

void test_blob(const void* data, size_t size)
{
    extern const void* __oe_get_blob_base(void);
    extern size_t __oe_get_blob_size(void);
    const void* blob_data;
    size_t blob_size;

    blob_data = __oe_get_blob_base();
    blob_size = __oe_get_blob_size();

    OE_TEST(data != NULL);
    OE_TEST(blob_data != NULL);
    OE_TEST(blob_size == size);
    OE_TEST(memcmp(blob_data, data, size) == 0);

    printf("%.*s\n", (int)blob_size, blob_data);

    oe_host_printf("=== passed all tests (enclave)\n");
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* AllowDebug */
    1024, /* HeapPageCount */
    1024, /* StackPageCount */
    2);   /* TCSCount */

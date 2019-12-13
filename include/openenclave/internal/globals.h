// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_GLOBALS_H
#define _OE_GLOBALS_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/types.h>
#include <openenclave/internal/types.h>

OE_EXTERNC_BEGIN

/* Enclave */
const void* __oe_get_enclave_base(void);
size_t __oe_get_enclave_size(void);
const void* __oe_get_enclave_elf_header(void);

/* Reloc */
const void* __oe_get_reloc_base(void);
const void* __oe_get_reloc_end(void);
size_t __oe_get_reloc_size(void);

/* blob data (from the .oeblob section) */
const void* __oe_get_blob_base(void);
const void* __oe_get_blob_end(void);
size_t __oe_get_blob_size(void);

/* Heap */
const void* __oe_get_heap_base(void);
const void* __oe_get_heap_end(void);
size_t __oe_get_heap_size(void);

/* The enclave handle passed by host during initialization */
extern oe_enclave_t* oe_enclave;

uint64_t oe_get_base_heap_page(void);
uint64_t oe_get_num_heap_pages(void);
uint64_t oe_get_num_pages(void);

OE_EXTERNC_END

#endif /* _OE_GLOBALS_H */

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/internal/elf.h>
#include <openenclave/internal/files.h>
#include <stdio.h>
#include <stdlib.h>

static const char* arg0;

OE_PRINTF_FORMAT(1, 2)
static void _err(const char* format, ...)
{
    fprintf(stderr, "\n");

    fprintf(stderr, "%s: error: ", arg0);

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n\n");

    exit(1);
}

int write_file(const char* path, const void* data, size_t size)
{
    FILE* os;

    if (!(os = fopen(path, "wb")))
        return -1;

    if (fwrite(data, 1, size, os) != size)
        return -1;

    fclose(os);

    return 0;
}

// clang-format off
static const char USAGE[] =
"\n"
"Usage: %s ENCLAVE BLOB\n"
"\n"
"Where:\n"
"    ENCLAVE - name of an enclave image file.\n"
"    BLOB - name of blob file.\n"
"\n"
"Adds a new section named '.oeblob' to an enclave image file that contains\n"
"the blob file. The enclave loader includes this section in the enclave code\n"
"measurement (MRENCLAVE). Enclaves can access the blob at runtime through the\n"
"the following definitions:\n"
"\n"
"    extern void* oe_blob_ptr;\n"
"    extern size_t oe_size;\n"
"\n"
"\n";
// clang-format on

int main(int argc, const char* argv[])
{
    int ret = 1;
    const char* enclave;
    const char* blob;
    elf64_t elf;
    bool loaded = false;
    void* data = NULL;
    size_t size;
    const char SECTION_NAME[] = ".oeblob";

    arg0 = argv[0];

    /* Check usage. */
    if (argc != 3)
    {
        fprintf(stderr, USAGE, argv[0]);
        goto done;
    }

    /* Disable logging noise to standard output. */
    setenv("OE_LOG_LEVEL", "NONE", 1);

    /* Collect the options. */
    enclave = argv[1];
    blob = argv[2];

    /* Load the ELF-64 object */
    {
        if (elf64_load(enclave, &elf) != 0)
            _err("failed to load enclave image: %s", enclave);

        loaded = true;
    }

    /* Load the blob. */
    if (__oe_load_file(blob, 0, &data, &size) != OE_OK)
        _err("failed to load blob file: %s", blob);

    /* Remove the section if it already exists. */
    {
        unsigned char* secdata;
        size_t secsize;

        if (elf64_find_section(&elf, SECTION_NAME, &secdata, &secsize) == 0)
        {
            if (elf64_remove_section(&elf, SECTION_NAME) != 0)
            {
                _err("failed to remove '%s' section", SECTION_NAME);
            }
        }
    }

    /* Add the new section. */
    if (elf64_add_section(&elf, SECTION_NAME, SHT_NOTE, data, size) != 0)
    {
        _err("failed to add '%s' section", SECTION_NAME);
    }

    /* Rewrite the enclave file. */
    if (write_file(enclave, elf.data, elf.size) != 0)
    {
        _err("failed to write enclave image: '%s'", enclave);
    }

    ret = 0;

done:

    if (loaded)
        elf64_unload(&elf);

    if (data)
        free(data);

    return ret;
}

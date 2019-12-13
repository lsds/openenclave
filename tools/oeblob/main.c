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
"Usage: %s BLOB INFILE OUTFILE\n"
"\n"
"Where:\n"
"    BLOBFILE - name of the blob file.\n"
"    INFILE - the input enclave image file.\n"
"    OUTFILE - the output enclave image file.\n"
"\n"
"Adds a blob to a new section of an enclave image file. The new section is\n"
"named '.oeblob' of type PROGBITS. The enclave loader includes this section\n"
"in the enclave code measurement (MRENCLAVE). Enclaves can access the blob\n"
"at runtime by calling these functions.\n"
"\n"
"    extern const void* __oe_get_blob_base(void);\n"
"    extern const void* __oe_get_blob_end(void);\n"
"    extern size_t __oe_get_blob_size(void);\n"
"\n"
"\n";
// clang-format on

int main(int argc, const char* argv[])
{
    int ret = 1;
    const char* infile;
    const char* outfile;
    const char* blobfile;
    elf64_t elf;
    bool loaded = false;
    void* data = NULL;
    size_t size;
    const char SECTION_NAME[] = ".oeblob";

    arg0 = argv[0];

    /* Check usage. */
    if (argc != 4)
    {
        fprintf(stderr, USAGE, argv[0]);
        goto done;
    }

    /* Disable logging noise to standard output. */
    setenv("OE_LOG_LEVEL", "NONE", 1);

    /* Collect the options. */
    blobfile = argv[1];
    infile = argv[2];
    outfile = argv[3];

    /* Load the ELF-64 object */
    {
        if (elf64_load(infile, &elf) != 0)
            _err("failed to load enclave image: %s", infile);

        loaded = true;
    }

    /* Load the blob file into memory. */
    if (__oe_load_file(blobfile, 0, &data, &size) != OE_OK)
        _err("failed to load blob file: %s", blobfile);

    /* Fail if the section already exists. */
    {
        unsigned char* secdata;
        size_t secsize;

        if (elf64_find_section(&elf, SECTION_NAME, &secdata, &secsize) == 0)
            _err("section already exists: '%s'", SECTION_NAME);
    }

    /* Add the new section. */
    if (elf64_add_section(&elf, SECTION_NAME, SHT_PROGBITS, data, size) != 0)
        _err("failed to add section: '%s'", SECTION_NAME);

    /* Rewrite the enclave file. */
    if (write_file(outfile, elf.data, elf.size) != 0)
        _err("failed to write enclave image: '%s'", outfile);

    ret = 0;

done:

    if (loaded)
        elf64_unload(&elf);

    if (data)
        free(data);

    return ret;
}

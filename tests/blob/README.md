blob test
=========

This test verifies the .oeblob section loading feature. Enclave image files may
optionally include an .oeblob section. If present, the section is loaded and
measured during enclave loading.  Enclaves may access this section at runtime
with the following functions.

```
extern const void* __oe_get_blob_base(void);
extern size_t __oe_get_blob_size(void);
```

This test uses the **objcopy** command to add an .oeblob section. For example:

```
$ objcopy --add-section .oeblob=blob.txt --set-section-flags .oeblob=noload,readonly blob_enc blob_enc_with_blob
```

The host program loads the **blob_enc_with_blob** enclave and then passes it
a copy of the blob file. The enclave compares the expected blob contents with
the actual contents of the .oeblob section.

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

void ve_call_fini_functions(void)
{
    void (**fn)(void);
    extern void (*__fini_array_start)(void);
    extern void (*__fini_array_end)(void);

    for (fn = &__fini_array_end - 1; fn >= &__fini_array_start; fn--)
    {
        (*fn)();
    }
}

void __libc_csu_fini(void)
{
    ve_call_fini_functions();
}

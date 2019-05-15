// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#define _GNU_SOURCE

// clang-format off
#include <openenclave/enclave.h>
// clang-format on

#include <openenclave/internal/posix/device.h>
#include <openenclave/internal/posix/raise.h>
#include <openenclave/corelibc/sys/socket.h>
#include <openenclave/corelibc/netdb.h>
#include <openenclave/internal/posix/resolver.h>
#include <openenclave/bits/safemath.h>
#include <openenclave/internal/calls.h>
#include <openenclave/internal/thread.h>
#include <openenclave/internal/print.h>
#include <openenclave/corelibc/stdlib.h>
#include <openenclave/corelibc/string.h>
#include <openenclave/bits/module.h>
#include <openenclave/internal/trace.h>
#include "posix_t.h"

/*
**==============================================================================
**
** hostresolv operations:
**
**==============================================================================
*/

// The host resolver is not actually a device in the file descriptor sense.

#define RESOLV_MAGIC 0x536f636b

typedef struct _resolv
{
    struct _oe_resolver base;
    uint32_t magic;
} resolv_t;

static resolv_t* _cast_resolv(const oe_resolver_t* device)
{
    resolv_t* resolv = (resolv_t*)device;

    if (resolv == NULL || resolv->magic != RESOLV_MAGIC)
        return NULL;

    return resolv;
}

static resolv_t _hostresolv;

static ssize_t _hostresolv_getnameinfo(
    oe_resolver_t* dev,
    const struct oe_sockaddr* sa,
    oe_socklen_t salen,
    char* host,
    oe_socklen_t hostlen,
    char* serv,
    oe_socklen_t servlen,
    int flags)
{
    int ret = OE_EAI_FAIL;

    OE_UNUSED(dev);

    oe_errno = 0;

    if (oe_posix_getnameinfo_ocall(
            &ret, sa, salen, host, hostlen, serv, servlen, flags) != OE_OK)
    {
        goto done;
    }

done:

    return ret;
}

static int _hostresolv_getaddrinfo(
    oe_resolver_t* resolv,
    const char* node,
    const char* service,
    const struct oe_addrinfo* hints,
    struct oe_addrinfo** res)
{
    int ret = OE_EAI_FAIL;
    uint64_t handle = 0;
    struct oe_addrinfo* head = NULL;
    struct oe_addrinfo* tail = NULL;
    struct oe_addrinfo* p = NULL;

    OE_UNUSED(resolv);

    if (res)
        *res = NULL;

    if (!res)
        OE_RAISE_ERRNO(OE_EINVAL);

    /* Get the handle for enumerating addrinfo structures. */
    {
        if (oe_posix_getaddrinfo_open_ocall(&handle, node, service, hints) !=
            OE_OK)
        {
            OE_RAISE_ERRNO(OE_EINVAL);
        }

        if (!handle)
            OE_RAISE_ERRNO(oe_errno);
    }

    /* Enumerate addrinfo structures. */
    for (;;)
    {
        int retval = 0;
        size_t canonnamelen = 0;

        if (!(p = oe_calloc(1, sizeof(struct oe_addrinfo))))
            OE_RAISE_ERRNO(OE_ENOMEM);

        /* Determine required size ai_addr and ai_canonname buffers. */
        if (oe_posix_getaddrinfo_read_ocall(
                &retval,
                handle,
                &p->ai_flags,
                &p->ai_family,
                &p->ai_socktype,
                &p->ai_protocol,
                p->ai_addrlen,
                &p->ai_addrlen,
                NULL,
                canonnamelen,
                &canonnamelen,
                NULL) != OE_OK)
        {
            OE_RAISE_ERRNO(OE_EINVAL);
        }

        /* If this is the final element in the enumeration. */
        if (retval == 1)
            break;

        /* Expecting that addr and canonname buffers were too small. */
        if (retval != -1 || oe_errno != OE_ENAMETOOLONG)
            OE_RAISE_ERRNO(OE_EINVAL);

        if (p->ai_addrlen && !(p->ai_addr = oe_calloc(1, p->ai_addrlen)))
            OE_RAISE_ERRNO(OE_ENOMEM);

        if (canonnamelen && !(p->ai_canonname = oe_calloc(1, canonnamelen)))
            OE_RAISE_ERRNO(OE_ENOMEM);

        if (oe_posix_getaddrinfo_read_ocall(
                &retval,
                handle,
                &p->ai_flags,
                &p->ai_family,
                &p->ai_socktype,
                &p->ai_protocol,
                p->ai_addrlen,
                &p->ai_addrlen,
                p->ai_addr,
                canonnamelen,
                &canonnamelen,
                p->ai_canonname) != OE_OK)
        {
            OE_RAISE_ERRNO(OE_EINVAL);
        }

        /* Append to the list. */
        if (tail)
        {
            tail->ai_next = p;
            tail = p;
        }
        else
        {
            head = p;
            tail = p;
        }

        p = NULL;
    }

    /* Close the enumeration. */
    if (handle)
    {
        int retval = -1;

        if (oe_posix_getaddrinfo_close_ocall(&retval, handle) != OE_OK)
            OE_RAISE_ERRNO(OE_EINVAL);

        handle = 0;

        if (retval != 0)
            OE_RAISE_ERRNO(oe_errno);
    }

    /* If the list is empty. */
    if (!head)
        OE_RAISE_ERRNO(OE_EINVAL);

    *res = head;
    head = NULL;
    tail = NULL;
    ret = 0;

done:

    if (handle)
    {
        int retval;
        oe_posix_getaddrinfo_close_ocall(&retval, handle);
    }

    if (head)
        oe_freeaddrinfo(head);

    if (p)
        oe_freeaddrinfo(p);

    return ret;
}

static int _hostresolv_shutdown(oe_resolver_t* resolv_)
{
    int ret = -1;
    resolv_t* resolv = _cast_resolv(resolv_);

    oe_errno = 0;

    if (!resolv)
        OE_RAISE_ERRNO(OE_EINVAL);

    if (oe_posix_shutdown_resolver_device_ocall(&ret) != OE_OK)
        OE_RAISE_ERRNO(OE_EINVAL);

    /* Release the resolv_ object. */
    oe_free(resolv);
    ret = 0;
done:
    return ret;
}

// clang-format off
static oe_resolver_ops_t _ops =
{
    .getaddrinfo = _hostresolv_getaddrinfo,
    .getnameinfo = _hostresolv_getnameinfo,
    .shutdown = _hostresolv_shutdown
};
// clang-format on

// clang-format off
static resolv_t _hostresolv =
{
    .base.type = OE_RESOLVER_HOST,
    .base.ops = &_ops,
    .magic = RESOLV_MAGIC
};
// clang-format on

static oe_once_t _once = OE_ONCE_INITIALIZER;
static bool _loaded;

static void _load_once(void)
{
    oe_result_t result = OE_FAILURE;
    oe_resolver_t* resolver = &_hostresolv.base;

    if (oe_register_resolver(2, resolver) != 0)
        OE_RAISE_ERRNO(oe_errno);

    result = OE_OK;

done:

    if (result == OE_OK)
        _loaded = true;
}

oe_result_t oe_load_module_host_resolver(void)
{
    if (oe_once(&_once, _load_once) != OE_OK || !_loaded)
        return OE_FAILURE;

    return OE_OK;
}
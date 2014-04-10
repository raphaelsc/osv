/*-
 * Copyright (c) 2000-2013 Mark R V Murray
 * Copyright (c) 2013 Arthur Mesh <arthurmesh@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "drivers/urandom.hh"

#include <osv/device.h>
#include <osv/uio.h>
#include <sys/selinfo.h>
#include <sys/random.h>
#include <sys/param.h>

#include <dev/random/randomdev.h>
#include <dev/random/randomdev_soft.h>
#include <dev/random/random_adaptors.h>

namespace urandomdev {

struct urandom_device_priv {
    urandom_device* drv;
};

static urandom_device_priv *to_priv(device *dev)
{
    return reinterpret_cast<urandom_device_priv*>(dev->private_data);
}

static int
urandom_read(struct device *dev, struct uio *uio, int ioflags)
{
    int c, error = 0;
    void *random_buf;
#ifdef TODO
    /* Blocking logic */
    if (!random_adaptor->seeded)
        error = (*random_adaptor->block)(ioflags);
#endif
    /* The actual read */
    if (!error) {
        random_buf = (void *)malloc(PAGE_SIZE);

        while (uio->uio_resid > 0 && !error) {
            c = MIN(uio->uio_resid, PAGE_SIZE);
            c = (*random_adaptor->read)(random_buf, c);
            error = uiomove(random_buf, c, uio);
        }
        /* Finished reading; let the source know so it can do some
         * optional housekeeping */
        (*random_adaptor->read)(NULL, 0);

        free(random_buf);
    }

    return (error);
}

static int
urandom_write(struct device *dev, struct uio *uio, int ioflags)
{
    /* We used to allow this to insert userland entropy.
     * We don't any more because (1) this so-called entropy
     * is usually lousy and (b) its vaguely possible to
     * mess with entropy harvesting by overdoing a write.
     * Now we just ignore input like /dev/null does.
     */
    uio->uio_resid = 0;

    return (0);
}

static struct devops urandom_device_devops {
    no_open,
    no_close,
    urandom_read,
    urandom_write,
    no_ioctl,
    no_devctl,
};

struct driver urandom_device_driver = {
    "urandom",
    &urandom_device_devops,
    sizeof(struct urandom_device_priv),
};

urandom_device::urandom_device()
{
    struct urandom_device_priv *prv;

    (random_adaptor->init)();
    _urandom_dev = device_create(&urandom_device_driver, "urandom", D_CHR);
    prv = to_priv(_urandom_dev);
    prv->drv = this;
}

urandom_device::~urandom_device()
{
    (random_adaptor->deinit)();
    device_destroy(_urandom_dev);
}

void urandomdev_init()
{
    static int random_initialized = 0;

    if (random_initialized) {
        printf("urandom: <%s> already initialized\n",
            random_adaptor->ident);
        return;
    }

    new urandom_device();
    printf("urandom: <%s> initialized\n", random_adaptor->ident);
    random_initialized = 1;
}

}

/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef URANDOM_DEVICE_H
#define URANDOM_DEVICE_H

#include <osv/device.h>
#include <osv/types.h>
#include <memory>

namespace urandomdev {

class urandom_device {
public:

    urandom_device();
    virtual ~urandom_device();

private:

    device* _urandom_dev;
};

void urandomdev_init();

}

#endif

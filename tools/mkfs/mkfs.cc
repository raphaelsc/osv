/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <assert.h>
#include <osv/device.h>
#include <osv/run.hh>
#include <fs/vfs/vfs.h>
#include <iostream>
#include "drivers/zfs.hh"

using namespace osv;
using namespace std;

// Guarantee that run_cmd will not inline so that the library resources
// will be surely released at the end.
void __attribute__ ((noinline))
run_cmd(const char *cmdpath, vector<string> args)
{
    int ret;
    auto ok = run(cmdpath, args, &ret);
    assert(ok && ret == 0);
}

void mkfs()
{
    // Create zfs device, then /etc/mnttab which is required by libzfs
    zfsdev::zfsdev_init();

    // Manually create the file required by libzfs. Later on, it's
    // added into the file system by the upload manifest phase
    mkdir("/etc", 0755);
    int fd = creat("/etc/mnttab", 0644);
    assert(fd != -1);
    close(fd);

    // Create zpool
    run_cmd("/zpool.so",
        {"zpool", "create", "-f", "-R", "/zfs", "osv", "/dev/vblk0.1"});

    // Create zfs on top of the created zpool
    run_cmd("/zfs.so", {"zfs", "create", "osv/zfs"});

#if 0
    // Enable dedup property in the file system to avoid storing redundant
    // copies of data
    run_cmd("/zfs.so", {"zfs", "set", "dedup=on", "osv"});
#endif
#if 0
    // Enable compression
    run_cmd("/zfs.so", {"zfs", "set", "compression=on", "osv"});
#endif
}

int main(int ac, char** av)
{
    cout << "Running mkfs...\n";
    mkfs();
    sync();
}


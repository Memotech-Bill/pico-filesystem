// pfs_dev_kbd.h - USB keyboard driver for PFS
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_DEV_KBD_H
#define PFS_DEV_KBD_H

#include <pfs.h>
#include <pfs_dev_keymap.h>

struct pfs_device *pfs_dev_kbd_fetch (PFS_DEV_KEYMAP *keymap);

#endif

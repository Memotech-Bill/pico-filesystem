// pfs_dev_keymap.h - Definition of keyboard mapping
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_DEV_KEYMAP_H
#define PFS_DEV_KEYMAP_H

typedef struct
    {
    int     nkey;
    struct
        {
        char    lower;
        char    upper;
        } key[];
    } PFS_DEV_KEYMAP;

#endif

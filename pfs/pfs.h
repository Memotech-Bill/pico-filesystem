/* Public interface to PFS */
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_H
#define PFS_H

struct pfs_pfs;
struct lfs_config;
struct pfs_device;

int pfs_init (void);
int pfs_mount (struct pfs_pfs *pfs, const char *psMount);
int ffs_pico_createcfg (struct lfs_config *cfg, int offset, int size);
struct pfs_pfs *pfs_ffs_create (const struct lfs_config *cfg);
struct pfs_pfs *pfs_fat_create (void);
struct pfs_pfs *pfs_dev_fetch (void);
int pfs_mknod (const char *name, int mode, const struct pfs_device *dev);
#endif

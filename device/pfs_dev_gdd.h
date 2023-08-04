// pfs_dev_gdd.c - A simple generic driver for a display device
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_DEV_DISPLAY_H
#define PFS_DEV_DISPLAY_H

#ifndef GIO_OUTPUT_H
#define GIO_OUTPUT_H
typedef void (*GIO_OUTPUT_RTN) (char ch);
#endif

struct pfs_device *pfs_dev_gdd_create (GIO_OUTPUT_RTN display);

#endif

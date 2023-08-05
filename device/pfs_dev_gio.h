// pfs_dev_gio.h - A generic PFS input / output device driver
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_DEV_GIO_H
#define PFS_DEV_GIO_H

#include <pfs.h>
#include <ioctl.h>

// For backwards compatibility
#define GIO_M_MODE      IOC_RQ_MODE
#define GIO_M_PURGE     IOC_RQ_PURGE
#define GIO_M_COUNT     IOC_RQ_COUNT
#define GIO_M_TOUT      IOC_RQ_TOUT
#define GIO_M_SCFG      IOC_RQ_SCFG

#ifndef GIO_OUTPUT_H
#define GIO_OUTPUT_H
typedef void (*GIO_OUTPUT_RTN) (char ch);
#endif

int pfs_dev_gio_input (struct pfs_device *gio, char ch);
struct pfs_device *pfs_dev_gio_create (GIO_OUTPUT_RTN output, int ndata, int mode);

#endif

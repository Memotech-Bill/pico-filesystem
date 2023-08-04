// pfs_dev_gio.h - A generic PFS input / output device driver
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_DEV_GIO_H
#define PFS_DEV_GIO_H

// Modes specifying when a read request will return
#define GIO_M_FULL      0x00000             // Only return when the buffer is full
#define GIO_M_NBLOCK    0x10000             // Never block waiting for input
#define GIO_M_ANY       0x20000             // Return as soon as at least one character input
#define GIO_M_CHR       0x40000             // Return when specified character received
#define GIO_M_CR        (GIO_M_CHR | '\r')  // Return when Carrage Return character received
#define GIO_M_LF        (GIO_M_CHR | '\n')  // Return when Line Feed character received
#define GIO_M_TLF       0x80000             // Replace terminating character by Line Feed

#ifndef GIO_OUTPUT_H
#define GIO_OUTPUT_H
typedef void (*GIO_OUTPUT_RTN) (char ch);
#endif

int pfs_dev_gio_input (struct pfs_device *gio, char ch);
struct pfs_device *pfs_dev_gio_create (GIO_OUTPUT_RTN output, int ndata, int mode);

#endif

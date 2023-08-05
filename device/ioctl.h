// ioctl.h - Input / Output controls for PFS devices
// Note: These have no resemblance to Linix / Windows IOCTLs
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef IOCTL_H
#define IOCTL_H

#define IOC_RQ_MODE     1                       // Set mode bits
#define IOC_RQ_PURGE    2                       // Purge all characters from receive buffer
#define IOC_RQ_COUNT    3                       // Get count of characters in receive buffer
#define IOC_RQ_TOUT     4                       // Set timeout in microseconds
#define IOC_RQ_SCFG     5                       // Set serial configuration

// Modes specifying when a read request will return
#define IOC_MD_FULL      0x00000                // Only return when the buffer is full
#define IOC_MD_NBLOCK    0x10000                // Never block waiting for input
#define IOC_MD_ANY       0x20000                // Return as soon as at least one character input
#define IOC_MD_CHR       0x40000                // Return when specified character received
#define IOC_MD_CR       (IOC_MD_CHR | '\r')     // Return when Carrage Return character received
#define IOC_MD_LF       (IOC_MD_CHR | '\n')     // Return when Line Feed character received
#define IOC_MD_TLF       0x80000                // Replace terminating character by Line Feed
#define IOC_MD_ECHO     0x100000                // Echo the input characters to the output

int ioctl (int fd, long request, void *argp);

#endif

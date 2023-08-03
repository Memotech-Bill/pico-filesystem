//  sconfig.h - Specify details of serial port connection
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef H_SCONFIG
#define H_SCONFIG

typedef struct
    {
    int     baud;
    int     parity;
    int     data;
    int     stop;
    int     tx;
    int     rx;
    int     cts;
    int     rts;
    } SERIAL_CONFIG;

#endif

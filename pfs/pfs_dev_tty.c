// pfs_dev_tty.c - A PFS character device using the Pico default console
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <sys/errno.h>
#include <pico/stdio.h>
#include <pfs_private.h>

#ifndef STATIC
#define STATIC  static
#endif

STATIC struct pfs_file *tty_open (const struct pfs_device *dev, const char *name, int oflags);
STATIC int tty_read (struct pfs_file *fd, char *buffer, int length);
STATIC int tty_write (struct pfs_file *fd, char *buffer, int length);
STATIC int tty_isatty (struct pfs_file *fd);

STATIC const struct pfs_device s_tty =
    {
    tty_open
    };

STATIC const struct pfs_v_file tty_v_file =
    {
    NULL,           // close
    tty_read,       // read
    tty_write,      // write
    NULL,           // lseek
    NULL,           // fstat
    tty_isatty,     // isatty
    NULL            // ioctl
    };

STATIC struct pfs_file *tty_open (const struct pfs_device *dev, const char *name, int oflags)
    {
    struct pfs_file *tty = (struct pfs_file *) malloc (sizeof (struct pfs_file));
    if ( tty == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    tty->entry = &tty_v_file;
    tty->pfs = NULL;
    tty->pn = NULL;
    return tty;
    }

STATIC int tty_read (struct pfs_file *fd, char *buffer, int length)
    {
    // return stdio_get_until (buffer, length, at_the_end_of_time);
    for (int i = 0; i < length; ++i)
        {
        int ch = PICO_ERROR_TIMEOUT;
        while (ch == PICO_ERROR_TIMEOUT)
            {
            ch = getchar_timeout_us (0xFFFFFFFF);
            }
        buffer[i] = (char) ch;
        }
    return length;
    }

STATIC int tty_write (struct pfs_file *fd, char *buffer, int length)
    {
    // stdio_put_string (buffer, length, false, false);
    for (int i = 0; i < length; ++i) putchar_raw (buffer[i]);
    return length;
    }

STATIC int tty_isatty (struct pfs_file *fd)
    {
    return 1;
    }

const struct pfs_device *pfs_dev_tty_fetch (void)
    {
    return &s_tty;
    }

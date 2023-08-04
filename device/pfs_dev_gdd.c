// pfs_dev_gdd.c - A simple generic driver for a output device
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <pfs_private.h>
#include <../device/pfs_dev_gdd.h>

#ifndef STATIC
#define STATIC  static
#endif

STATIC struct pfs_file *gdd_open (const struct pfs_device *dev, const char *name, int oflags);
STATIC int gdd_write (struct pfs_file *fd, char *buffer, int length);

struct gdd_device
    {
    struct pfs_file * (*open) (const struct pfs_device *dev, const char *name, int oflags);
    GIO_OUTPUT_RTN output;
    };

STATIC const struct pfs_v_file gdd_v_file =
    {
    NULL,           // close
    NULL,           // read
    gdd_write,      // write
    NULL,           // lseek
    NULL,           // fstat
    NULL,           // isatty
    NULL            // ioctl
    };

STATIC struct pfs_file *gdd_open (const struct pfs_device *dev, const char *name, int oflags)
    {
    if ( ( oflags & O_ACCMODE ) != O_WRONLY )
        {
        pfs_error (EACCES);
        return NULL;
        }
    struct pfs_file *gdd = (struct pfs_file *) malloc (sizeof (struct pfs_file));
    if ( gdd == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    gdd->entry = &gdd_v_file;
    gdd->pfs = (struct pfs_pfs *) dev;
    gdd->pn = NULL;
    return gdd;
    }

STATIC int gdd_write (struct pfs_file *fd, char *buffer, int length)
    {
    struct gdd_device *gdd = (struct gdd_device *) fd->pfs;
    for (int i = 0; i < length; ++i) gdd->output (buffer[i]);
    return length;
    }

struct pfs_device *pfs_dev_gdd_create (GIO_OUTPUT_RTN output)
    {
    struct gdd_device *gdd = (struct gdd_device *) malloc (sizeof (struct gdd_device));
    if ( gdd == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    gdd->open = gdd_open;
    gdd->output = output;
    return (struct pfs_device *) gdd;
    }

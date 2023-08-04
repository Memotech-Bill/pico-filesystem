// pfs_dev_gio.c - A generic PFS input / output device driver
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <pico/sync.h>
#include <pfs_private.h>
#include <pfs_dev_gio.h>

#ifndef STATIC
#define STATIC  static
#endif

STATIC struct pfs_file *gio_open (const struct pfs_device *dev, const char *name, int oflags);
STATIC int gio_read (struct pfs_file *fd, char *buffer, int length);
STATIC int gio_write (struct pfs_file *fd, char *buffer, int length);

struct pfs_dev_gio
    {
    struct pfs_file *   (*open)(const struct pfs_device *dev, const char *name, int oflags);
    GIO_OUTPUT_RTN      output;
    int                 mode;
    int                 ndata;
    int                 rptr;
    int                 wptr;
    char                data[];
    };

STATIC const struct pfs_v_file gio_v_file =
    {
    NULL,           // close
    gio_read,       // read
    gio_write,      // write
    NULL,           // lseek
    NULL,           // fstat
    NULL,           // isatty
    NULL,           // ioctl
    };

int pfs_dev_gio_input (struct pfs_device *dev, char ch)
    {
    struct pfs_dev_gio *gio = (struct pfs_dev_gio *) dev;
    int wend = ( gio->rptr - 1 ) & ( gio->ndata - 1 );
    if ( gio->wptr == wend ) return -1;
    gio->data[gio->wptr] = ch;
    gio->wptr = (++gio->wptr) & ( gio->ndata - 1 );
    if ( gio->wptr == wend ) return 1;
    return 0;
    }

STATIC int gio_read (struct pfs_file *fd, char *buffer, int length)
    {
    char *bptr = buffer;
    struct pfs_dev_gio *gio = (struct pfs_dev_gio *) fd->pfs;
    int nread = 0;
    while (length > 0)
        {
        if ( gio->rptr == gio->wptr )
            {
            if ( gio->mode == GIO_M_NBLOCK ) break;
            if (( gio->mode == GIO_M_ANY ) && ( nread > 0 )) break;
            }
        while ( gio->rptr == gio->wptr )
            {
            __wfi ();
            }
        *bptr = gio->data[gio->rptr];
        gio->rptr = (++gio->rptr) & ( gio->ndata - 1 );
        ++nread;
        --length;
        if (( gio->mode & GIO_M_CHR ) && ( *bptr == (gio->mode & 0xFF) ))
            {
            if ( gio->mode & GIO_M_TLF ) *bptr = '\n';
            break;
            }
        ++bptr;
        }
    return nread;
    }

STATIC int gio_write (struct pfs_file *fd, char *buffer, int length)
    {
    struct pfs_dev_gio *gio = (struct pfs_dev_gio *) fd->pfs;
    for (int i = 0; i < length; ++i) gio->output (buffer[i]);
    return length;
    }

STATIC struct pfs_file *gio_open (const struct pfs_device *dev, const char *name, int oflags)
    {
    struct pfs_dev_gio *giodev = (struct pfs_dev_gio *) dev;
    if (( giodev->output == NULL ) && (( oflags & O_ACCMODE ) != O_RDONLY ))
        {
        pfs_error (EACCES);
        return NULL;
        }
    struct pfs_file *gio = (struct pfs_file *) malloc (sizeof (struct pfs_file));
    if ( gio == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    gio->entry = &gio_v_file;
    gio->pfs = (struct pfs_pfs *) giodev;
    gio->pn = NULL;
    return gio;
    }

struct pfs_device *pfs_dev_gio_create (GIO_OUTPUT_RTN output, int ndata, int mode)
    {
    if ( ndata < 2 )
        {
        pfs_error (EINVAL);
        return NULL;
        }
    int nchk = ndata;
    while ( nchk > 1 )
        {
        if ( nchk & 1 )
            {
            pfs_error (EINVAL);
            return NULL;
            }
        nchk >>= 1;
        }
    struct pfs_dev_gio *gio = (struct pfs_dev_gio *) malloc (sizeof (struct pfs_dev_gio) + ndata);
    if ( gio == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    gio->open = gio_open;
    gio->output = output;
    gio->mode = mode;
    gio->ndata = ndata;
    gio->rptr = 0;
    gio->wptr = 0;
    return (struct pfs_device *) gio;
    }

/* pfs_dev.c - A PFS filesystem for hardware devices */
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <fcntl.h>
#include <pfs_private.h>

#ifndef STATIC
#define STATIC  static
#endif

STATIC struct pfs_file *dev_open (struct pfs_pfs *pfs, const char *fn, int oflag);
STATIC int dev_stat (struct pfs_pfs *pfs, const char *name, struct stat *buf);
STATIC void *dev_opendir (struct pfs_pfs *pfs, const char *name);
STATIC struct dirent *dev_readdir (void *dirp);

STATIC const struct pfs_v_pfs dev_v_pfs =
    {
    dev_open,
    dev_stat,
    NULL,           // rename
    NULL,           // delete
    NULL,           // mkdir
    NULL,           // rmdir
    dev_opendir,
    NULL            // chmod
    };

STATIC const struct pfs_v_dir dev_v_dir =
    {
    dev_readdir,
    NULL            // closedir
    };

struct dev_device
    {
    const struct dev_device *   next;
    const struct pfs_device *   dev;
    int                         nlen;
    char                        name[];
    };

STATIC struct dev_pfs
    {
    const struct pfs_v_pfs *    entry;
    const struct dev_device *   devs;
    }
s_dfs =
    {
    &dev_v_pfs,
    NULL
    };

struct dev_dir
    {
    const struct pfs_v_dir *    entry;
    struct dev_pfs *            dfs;
    int                         flags;
    struct pfs_mount *          m;
    struct dirent               de;
    const struct dev_device *   ddv;
    };

STATIC struct pfs_file *dev_open (struct pfs_pfs *pfs, const char *name, int oflag)
    {
    struct dev_pfs *dfs = (struct dev_pfs *) pfs;
    if ( name[0] == '/' ) ++name;
    for (const struct dev_device *ddv = dfs->devs; ddv != NULL; ddv = ddv->next )
        {
        if (( strncmp (name, ddv->name, ddv->nlen) == 0 )
            && (( name[ddv->nlen] == '\0' ) || ( ddv->name[ddv->nlen] == '*' )))
            return ddv->dev->open (ddv->dev, name, oflag);
        }
    pfs_error (ENXIO);
    return NULL;
    }

STATIC int dev_stat (struct pfs_pfs *pfs, const char *name, struct stat *buf)
    {
    struct dev_pfs *dfs = (struct dev_pfs *) pfs;
    if ( name[0] == '/' ) ++name;
    if ( name[0] == '\0' )
        {
        memset (buf, 0, sizeof (struct stat));
        buf->st_nlink = 1;
        buf->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR;
        return 0;
        }
    for (const struct dev_device *ddv = dfs->devs; ddv != NULL; ddv = ddv->next )
        {
        if (( strncmp (name, ddv->name, ddv->nlen) == 0 )
            && (( name[ddv->nlen] == '\0' ) || ( ddv->name[ddv->nlen] == '*' )))
            {
            memset (buf, 0, sizeof (struct stat));
            buf->st_nlink = 1;
            buf->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFCHR;
            return 0;
            }
        }
    return pfs_error (ENXIO);
    }
    

STATIC void *dev_opendir (struct pfs_pfs *pfs, const char *name)
    {
    struct dev_pfs *dfs = (struct dev_pfs *) pfs;
    struct dev_dir *dd = (struct dev_dir *) malloc (sizeof (struct dev_dir));
    if ( dd == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    dd->entry = &dev_v_dir;
    dd->dfs = dfs;
    dd->ddv = dfs->devs;
    return dd;
    }

STATIC struct dirent *dev_readdir (void *dirp)
    {
    struct dev_dir *dd = (struct dev_dir *) dirp;
    if ( dd->ddv == NULL ) return NULL;
    strncpy (dd->de.d_name, dd->ddv->name, NAME_MAX);
    dd->ddv = dd->ddv->next;
    return &dd->de;
    }

struct pfs_pfs *pfs_dev_fetch (void)
    {
    return (struct pfs_pfs *) &s_dfs;
    }

int pfs_mknod (const char *name, int mode, const struct pfs_device *dev)
    {
    struct dev_device *ddv = (struct dev_device *) malloc (sizeof (const struct dev_device) + strlen (name) + 1);
    if ( ddv == NULL ) return pfs_error (ENOMEM);
    ddv->next = s_dfs.devs;
    ddv->dev = dev;
    strcpy (ddv->name, name);
    ddv->nlen = strlen (name);
    if ( name[ddv->nlen - 1] == '*' ) --ddv->nlen;
    s_dfs.devs = ddv;
    return 0;
    }

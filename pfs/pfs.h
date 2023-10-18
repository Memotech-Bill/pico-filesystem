/* Public interface to PFS */
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PFS_H
#define PFS_H

struct pfs_pfs;
struct lfs_config;
struct pfs_device;

#ifdef __cplusplus
extern "C" {
#endif

// Initialises the pico filesystem. In particular configures stdin,
// stdout and stderr.

// The routine returns zero on success, or a negative error
// code on failure.

// There is no longer any need to explicitly call this routine as it
// is automatically called by either the mount routine, or the first
// of the NEWLIB _hook routines_ to be called.
int pfs_init (void);

// Mounts a volume and makes it available to the NEWLIB routines.

// *   pfs = Pointer to the volume definition from one of the above
//     routines.
// *   name = Pointer to the name of the mount point.

// The mount point name will form the first part of the absolute
// path name of any file on the volume. For clarity, the name may
// begin with a forward slash. If the forward slash is omitted,
// one will be assumed. The first volume to be mounted may be
// mounted as root ("/" or ""). All subsequent mounts must have a
// unique name. There must not be any other slashes in the mount
// point name.

// A mount point name will hide any file or folder
// with the same name on the root volume (if any). A directory
// listing of the root folder will list all the mount points,
// irrespecctive of whether there is a volume mounted as root.

// The routine returns zero on success, or a negative error
// code on failure.
int pfs_mount (struct pfs_pfs *pfs, const char *name);

// Initialises a lfs_config structure which is then used to inform
// littlefs where and how to write to Pico flash memory.

// *   cfg = Pointer to the lfs_config structure to initialise.
// *   offset = Starting address in flash memory to store the file data.
// *   size = Size (in bytes) of the data storage area.

// Returns zero if successful, or -1 if the offset specified is not a
// multiple of the FLASH_PAGE_SIZE (from the board definition file).
int ffs_pico_createcfg (struct lfs_config *cfg, int offset, int size);

// Creates a pfs_pfs structure which defines a flash storage volume
// to mount.

// *   cfg = Pointer to the lfs_config structure defining the flash
//     storage space.

// The contents of the cfg structure are copied in to the volume definition,
// so it is not necessary for the input structure to persist after this
// call has returned.
struct pfs_pfs *pfs_ffs_create (const struct lfs_config *cfg);

// Creates a pfs_pfs structure which defines an SD card storage volume
// to mount.

// The FATFS code maintains global state, so it is not currently
// possible to have multiple FAT volumes.
struct pfs_pfs *pfs_fat_create (void);

// There is only ever one device filesystem. This routine gets
// the pfs_pfs structure needed to mount the filesystem.

// The device_filesystem is initially empty until device drivers
// are attached to it.
struct pfs_pfs *pfs_dev_fetch (void);

// Attaches a device driver to the device_filesystem.

// *   name = Name for the device
// *   mode = Mode for the device (currently ignored)
// *   dev = Pointer to a pfs_device structure defining the
//     device

// The device name may end with a star (*), in which case it
// matches any name with the same beginning, and the entire
// name is passed to the driver, which may use the string to
// configure the device.
int pfs_mknod (const char *name, int mode, const struct pfs_device *dev);

#ifdef __cplusplus
}
#endif

#endif

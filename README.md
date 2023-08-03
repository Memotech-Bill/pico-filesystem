# Standard File Input / Output for the Pico SDK

The Pico SDK uses [NEWLIB](https://sourceware.org/newlib/)
to provide the C standard library. This library provides all
the functions in "stdio.h" such as `fopen` and `fprintf`.
However to make these work the library relies upon low level
_hook functions_ to implement the device specific code. The
_hook functions_ provided by the __pico-sdk__ only provide
support for __stdin__, __stdout__ and __stderr__, so attempting
to write to a file fails. Fortunately the __pico-sdk__ defines
the _hook functions_ as weak functions, so they can be overridden
by simply supplying alternate functions of the same name. That is
what this repository does.

## Basic Usage

### LFS Filesystem in Flash Memory

For the most common case of a single LFS filesystem in flash memory
add this repository to your CMakeLists.txt file with the commands:

````
    add_subdirectory (/path/to/pico-filesystem pfs)
    target_link_libraries(${PROJECT_NAME} flash_filesystem)

````

and then include the following code near the start of a program:

````
    #include <pfs.h>
    struct pfs_pfs *pfs;
    struct lfs_config cfg;
    ffs_pico_createcfg (&cfg, ROOT_OFFSET, ROOT_SIZE);
    pfs = pfs_ffs_create (&cfg);
    pfs_mount (pfs, "/");
````

ROOT_OFFSET and ROOT_SIZE specify the position and size of the LFS
file system in the Pico flash memory. Typical values might be:

````
#define ROOT_SIZE 0x100000
#define ROOT_OFFSET 0x100000
````

which uses the the upper half of the 2MB of Flash on a Pico.
These may be defined either in the C source code or CMake file
as convenient.

### FAT Filesystem on SD Card

Alternately, for a FAT filesystem on an SD card attached to the Pico
add this repository to your CMakeLists.txt file with the commands:

````
    add_subdirectory (/path/to/pico-filesystem pfs)
    target_link_libraries(${PROJECT_NAME} sdcard_filesystem)

````

and then include the following code near the start of a program:

````
    #include <pfs.h>
    struct pfs_pfs *pfs;
    pfs = pfs_fat_create ();
    pfs_mount (pfs, "/");
````

This has to be built with a board definition file which specifies
which Pico GPIO pins the SD card is attached to. The code uses an
SPI driver using PIO so almost any available GPIO pin numbers may be used.

## Code Structure

The code has been written as far as possible to be general purpose.
It can support multiple file systems. At present, drivers have been
written for LFS filesystems stored in the Pico flash memory, and
FAT filesystems stored on an SD card. The code is structured as:

````
                         +--------------+
                         | Main program |
                         +--------------+
                                |
                            +--------+
                            | NEWLIB |
                            +--------+
                                |
                        +----------------+
                        | pfs_filesystem |
                        +----------------+
                           /    |    \
           /--------------/     |     \----------------\
          /                     |                       \
+------------------+   +-------------------+   +-------------------+
| flash_filesystem |   | sdcard_filesystem |   | serial_filesystem |
+------------------+   +-------------------+   +-------------------+
         |                      |                        |
    +----------+            +-------+                    |
    | littlefs |            | FATFS |                    |
    +----------+            +-------+                    |
         |                      |                        |
    +----------+           +---------+                   |
    | ffs_pico |           | ff_disk |                   |
    +----------+           +---------+                   |
         |                      |                        |
+-------------------+      +---------+            +------------+
| Pico Flash Memory |      | SD Card |            | Pico UARTs |
+-------------------+      +---------+            +------------+
````

### pfs_filesystem

This provides the _hook functions_ needed by NEWLIB. It includes
implementations of __stdin__, __stdout__ and __stderr__ similar to
the __pico-sdk__. It also supports the mounting of one or more file
systems. Each file system is mounted at a named mount point such as
__/sdcard__. Optionally, the first mounted file system may be
mounted as root __/__. The filesystem to be mounted is described
by a __struct pfs_pfs__ pointer which is passed to the mount
routine. This code is device independent (not Pico specific)

### flash_filesystem

This provides the __struct pfs_pfs__ for the file system to be
mounted. It then translates between the generic file system functions
and those specific to LFS. This code is device independent (not Pico
specific)

### littlefs

This converts operations on files and directories into operations
on blocks of storage. It is this code that determines how directories
and files are laid out on storage in the LFS format. This code is
device independent (not Pico specific) and is (c) copyright ARM.

### ffs_pico

This provides the Pico specific routines needed to read, write and
erase blocks of flash memory to store the data. Note that while
writing or erasing data on flash memory, the other core, if running,
must not access flash. If the macro __PICO_MLOCK__ is defined then
the flash write and erase code is enclosed within calls to
__multicore_lockout_start_blocking()__ and
__multicore_lockout_end_blocking()__, which can be used to stall
the other core while flash is in use. See the __pico-sdk__
documentation for more details.

### sdcard_filesystem

This provides the __struct pfs_pfs__ for the file system to be
mounted. It then translates between the generic file system functions
and those specific to FATFS. This code is device independent (not Pico
specific)

### FATFS

This converts operations on files and directories into operations
on blocks of storage. It is this code that determines how directories
and files are laid out on storage in the FAT format. This code is
device independent (not Pico specific) and is (c) copyright ChaN.

### ff_disk

This provides the Pico specific routines needed to read, write and
erase SD card sectors to store the data. A single state machine on
PIO1 and two DMA channels are used to provide an SPI interface to
the SD card on any available GPIO pins. The board definition passed
to CMake must specify:

*   PICO_SD_CLK_PIN - Connect to SD card clock
*   PICO_SD_CMD_PIN - Connect to SD card command (data in)
*   PICO_SD_DAT0_PIN - Connect to SD card data 0 (data out)
*   PICO_SD_DAT_PIN_INCREMENT - Increment in GPIO pin number for
    subsequent data pins.

SD card pin "Data 3" must be connected to GPIO pin
(PICO_SD_DAT0_PIN + 3 * PICO_SD_DAT_PIN_INCREMENT) to provide SPI
Chip Select. If SD card pins "Data 1" and "Data 2" are connected
to the Pico, then specifying PICO_SD_DAT_PIN_COUNT will cause these
pins to be pulled high. If these pins are not connected to the Pico
then they must be wired to be pulled high.

### serial_filesystem

This provides one or more device nodes for serial input / output.
This code was developed for the
[Pico version of BBC BASIC](https://github.com/Memotech-Bill/PicoBB)
and is probably of less general interest.

## Application Programming Interface

There is very little API to this software, just enough to configure
and mount any file systems to use at the start of a program. After
that, the routines are used via the standard C interface for file
input / output and file and directory operations.

### int pfs_init (void)

Initialises the pico filesystem. In particular configures __stdin__,
__stdout__ and __stderr__.

The routine returns zero on success, or a negative error
code on failure.

There is no longer any need to explicitly call this routine as it
is automatically called by either the __mount__ routine, or the first
of the NEWLIB _hook routines_ to be called.

### int ffs_pico_createcfg (struct lfs_config *cfg, int offset, int size)

Initialises a __lfs_config__ structure which is then used to inform
littlefs where and how to write to Pico flash memory.

*   cfg = Pointer to the __lfs_config__ structure to initialise.
*   offset = Starting address in flash memory to store the file data.
*   size = Size (in bytes) of the data storage area.

Returns zero if successful, or -1 if the offset specified is not a
multiple of the FLASH_PAGE_SIZE (from the board definition file).

### struct pfs_pfs *pfs_ffs_create (const struct lfs_config *cfg)

Creates a __pfs_pfs__ structure which defines a flash storage volume
to mount.

*   cfg = Pointer to the __lfs_config__ structure defining the flash
    storage space.

The contents of the cfg structure are copied in to the volume definition,
so it is not necessary for the input structure to persist after this
call has returned.

### struct pfs_pfs *pfs_fat_create (void)

Creates a __pfs_pfs__ structure which defines an SD card storage volume
to mount.

The FATFS code maintains global state, so it is not currently
possible to have multiple FAT volumes.

### struct pfs_pfs *pfs_ser_create (void)

Creates a __pfs_pfs__ structure which defines a device node volume
for the serial ports.

### int pfs_mount (struct pfs_pfs *pfs, const char *psMount)

Mounts a volume and makes it available to the NEWLIB routines.

*   pfs = Pointer to the volume definition from one of the above
    routines.
*   psMount = Pointer to the name of the mount point.

The mount point name will form the first part of the absolute
path name of any file on the volume. For clarity, the name may
begin with a forward slash. If the forward slash is omitted,
one will be assumed. The first volume to be mounted may be
mounted as root ("/" or ""). All subsequent mounts must have a
unique name. There must not be any other slashes in the mount
point name.

A mount point name will hide any file or folder
with the same name on the root volume (if any). A directory
listing of the root folder will list all the mount points,
irrespecctive of whether there is a volume mounted as root.

The routine returns zero on success, or a negative error
code on failure.

## Error codes

*   0 - Success.
*   -2 - No memory for file handles
*   -3 - Failed to create __stdin__
*   -4 - Failed to create __stdout__
*   -5 - Failed to create __stderr__
*   -6 - Volume pointer is NULL
*   -7 - No memory for mount point
*   -8 - Slash in mount point name
*   -9 - Root mount is not the first mount
*   -10 - Duplicate mount point name

## Multiple filesystems

To have both Flash and SD Card filesystems, include both
`flash_filesystem` and `sdcard_filesystem` as link libraries in
the CMakeLists.txt file.

One filesystem may be mounted as root, as shown above. Additional
filesystems may be mounted at named mount points immediately below
root, for example as "/sdcard". In terms of parsing file names it
would not be difficult to allow filesystems to be mounted in
sub-level folders. What is difficult is making such mount points
appear in directory listings.

Should you wish to, it is perfectly possible to create multiple
flash volumes, occupying different areas of flash storage, and to
then mount these volumes at different mount points.

## Volume Drivers

To implement a driver for a new filesystem, it is probably easiest
to follow the example of the flash filesystem driver in
"flash/pfs_ffs.c". For descriptive purposes assume that you are
implementing "Your File System", and using the three letters "yfs"
to identify the code. Then in the source code ("pfs_yfs.c") there
should be:

1. Forward declarations of the functions you need to implement:

   ````
   struct pfs_file *yfs_open (struct pfs_pfs *pfs, const char *fn, int oflag);
   int yfs_close (struct pfs_file *pfs_fd);
   int yfs_read (struct pfs_file *pfs_fd, char *buffer, int length);
   int yfs_write (struct pfs_file *pfs_fd, char *buffer, int length);
   long yfs_lseek (struct pfs_file *pfs_fd, long pos, int whence);
   int yfs_fstat (struct pfs_file *pfs_fd, struct stat *buf);
   int yfs_isatty (struct pfs_file *fd);
   int yfs_stat (struct pfs_pfs *pfs, const char *name, struct stat *buf);
   int yfs_rename (struct pfs_pfs *pfs, const char *old, const char *new);
   int yfs_delete (struct pfs_pfs *pfs, const char *name);
   int yfs_mkdir (struct pfs_pfs *pfs, const char *pathname, mode_t mode);
   int yfs_rmdir (struct pfs_pfs *pfs, const char *pathname);
   void *yfs_opendir (struct pfs_pfs *pfs, const char *name);
   struct dirent *yfs_readdir (void *dirp);
   int yfs_closedir (void *dirp);
   int yfs_chmod (struct pfs_pfs *pfs, const char *pathname, mode_t mode);
    
   ````

2. Static constant structures providing vectors into your routines

   ````
   static const struct pfs_v_pfs yfs_v_pfs =
       {
       yfs_open,
       yfs_stat,
       yfs_rename,
       yfs_delete,
       yfs_mkdir,
       yfs_rmdir,
       yfs_opendir,
       yfs_chmod
       };
       
   static const struct pfs_v_file yfs_v_file =
       {
       yfs_close,
       yfs_read,
       yfs_write,
       yfs_lseek,
       yfs_fstat,
       };
    
   static const struct pfs_v_dir yfs_v_dir =
       {
       yfs_readdir,
       yfs_closedir,
       };
   ````

3. Definition of structures providing details of:

   * The filesystem:

     ````
     struct yfs_pfs
         {
         const struct pfs_v_pfs *    entry;  // = &yfs_v_pfs
         // Any data specific to your filesystem here
         };
     ````

   * An open file:

     ````
     struct yfs_file
         {
         const struct pfs_v_file *   entry;  // = &yfs_v_file
         struct yfs_pfs *            yfs;    // Pointer to the volume data
         const char *                pn;     // Full pathname of the file
         // Any data specific to an open file on your filesystem
         };
     ````

   * An open directory:

     ````
     struct yfs_dir
         {
         const struct pfs_v_dir *    entry;  // = &yfs_v_dir
         struct yfs_pfs *            yfs;    // Pointer to the volume data
         int                         flags;  // Used internally by _readdir (do not use)
         struct pfs_mount *          m;      // Used internally by _readdir (do not use)
         struct dirent               de;     // Buffer for storing results of directory search
         // Any data specific to an open directory on your filesystem
         };
     ````

4. Your implementation of the functions listed in (1). Points to note:

    * All input filenames are given as full paths (excluding the mount point name)
    * In the event of an error `pfs_err()` should be called with the appropriate
      error code from <sys/errno.h>, and return either NULL or -1 according to
      the return type of the function.
    * On success `yfs_open(...)` should allocate a `struct yfs_file` on the heap,
      populate the structure as required and return a pointer to the allocated
      structure. In the event of an error any allocated memory should be freed
      and a NULL pointer returned.
    * `yfs_opendir(...)` should be similar to `yfs_open(...)` with the structure
      it returns.
    * `yfs_close(...)` and `yfs_closedir(...)` should NOT free the memory associated
      with the pointers they are given. That is done within the PFS `_close(...)`
      and `_closedir(...)` routines.

5. A routine to allocate and populate an instance of `struct yfs_pfs`.
   This routine may take whatever parameters are necessary
   to specify the volume.

### LFS on other Media

The __littlefs__ code uses a `struct lfs_config` to specify the
routines that have to be called to read, write or erase the media
it wants to access, together with values defining properties of
the media. This structure is passed into the call to
`pfs_ffs_create(...)`. Thus in order to implement LFS on media
other than the Pico flash, it is only necessary to implement
the necessary media access routines and complete an appropriate
`struct lfs_config` to pass to `pfs_ffs_create(...)` to
create a volume for mounting.

Since LFS provides wear leveling, it might be useful to provide
LFS on an SD card as an alternative to FAT.

### FAT on other Media

Unfortunately the __FATFS__ code uses fixed routine names for
media access. So it is not so simple to be able to load volumes
on other media.

TODO: Look at makingg use of FATFS drive numbers, or at modifying
the FATFS code to use dynamic pointers to the media access routines.

## Implementation Notes

A couple of issues arose during the implementation:

1. The version of NEWLIB that gets installed on Raspberry Pi OS
   appears to have been built without `HAVE_RENAME` being
   defined. As a consequence of this, NEWLIB implements `rename`
   as a call to `_link` to create a new hard link to the file,
   followed by `_unlink` to remove the old link. Of course,
   neither LFS or FAT filesystems support hard links. Therefore,
   `_link` has to be implemented as a rename operation, and
   `_unlink` has to be fudged so that it does not error owing
   to the old file name no longer existing. This makes `rename`
   work, but it means you do not get the expected error if you
   actually attempt to create a hard link.

2. It is necessary to provide an override (replacement) for
   the NEWLIB version of "dirent.h". Otherwise, the NEWLIB
   standard version includes "sys/dirent.h". That, in turn,
   includes the following line which will cause a compilation
   failure:

````
#error "<dirent.h> not supported"
````
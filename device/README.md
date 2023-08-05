# Device Drivers for __pico-filesystem__

See (../README.md) for general details of __pico-filesystem__.

This folder contains software drivers for character devices that
can be opened, read from or written to using __stdio.h__ routines.

## Typical usage

Near the beginning of your code, attach one or more devices to
the device filesystem, then mount the filesystem.

````
        struct pfs_device *dev = pfs_dev_tty_fetch ();
        pfs_mknod ("tty0", 0, dev);
        // Repeat similar to above for other devices
        
        struct pfs_pfs *pfs = pfs_dev_fetch ();
        pfs_mount (pfs, "/dev");
````

## Available drivers

### tty driver

This driver provides input and output via the __pico_stdio__
interface which can use USB, UART or SemiHosting. It is,
by default, attached to the first three file handles.
Making it available via the __device_filesystem__ means that
these handles can be restored if they had been redirected to
other devices.

````
        struct pfs_device *pfs_dev_tty_fetch ();
````

The device definition for this device is statically allocated.
this call fetches a reference to the device definition.

In CMake it is included as part of pico_filesystem.

### Generic Output Driver

This driver makes it easy to support output only devices such
as displays. It is only necessary to provide one routine, which
writes single characters to the display. Typical usage for an
example VGA display:

````
#include <pfs.h>
#include <pfs_dev_gdd.h>

    // Forward definition of a routine to write a character to the screen
    void vga_put (char ch);

    // Attach the device to the filesystem
    struct pfs_device *vga = pfs_dev_gdd (vga_put);
    int ierr = pfs_mknod ("vga", 0, vga);

    // Mount the device filesystem
    struct pfs_pfs *pfs = pfs_dev_fetch ();
    pfs_mount (pfs, "/dev");
````

Note that devices created by this driver are output only, so write mode
must be selected when opening the device:

````
    FILE *f_vga = fopen ("/dev/vga", "w");
````

In CMake specify the __pfs_dev_gdd__ link library for this device driver.

### Generic Input / Output Driver

This driver makes it easy to implement input only devices or bidirectional
devices. The __pfs_device__ structure is created using the call:

````
struct pfs_device *pfs_dev_gio_create (GIO_OUTPUT_RTN output, int ndata, int mode);
````

* __output__ = Name of the routine to output characters written to
  the device, one character at a time. Set this to NULL for an input
  only device.

* __ndata__ = Size of the buffer to store received characters until
  they are read. This must be a power of 2.

* __mode__ = Value which determines when a read from the device will return:
  + __IOC_MD_FULL__     - Only return when the buffer is full
  + __IOC_MD_NBLOCK__   - Never block waiting for input
  + __IOC_MD_ANY__      - Return as soon as at least one character input
  + __IOC_MD_CHR__      - Return when specified character received
  + __IOC_MD_CR__       - Return when Carrage Return character received
  + __IOC_MD_LF__       - Return when Line Feed character received
  + __IOC_MD_TLF__      - Replace terminating character by Line Feed
  + __IOC_MD_ECHO__     - Echo input characters to output

As for the output only driver, a single routine which receives single characters
has to be supplied for outout using this device driver.

For input, characters will typically be received in an interrupt routine.
The device driver pointer returned by __pfs_def_gio_create__ should be saved and
available to the interrupt routine. The interrupt routine should call:

````
int pfs_dev_gio_input (struct pfs_device *gio, char ch);
````

to add characters to the buffer to await reading. The return value
from this routine is:

* 0 -  Character saved and more room in the buffer.
* +1 - Character saved and buffer now full.
* -1 - Buffer full, character not saved.

In CMake specify the __pfs_dev_gio__ link library for this device driver.

### UART driver

This driver provides support for either of the Pico hardware UARTs.
To create a __pfs_device__ for one of the UARTs call:

````
struct pfs_device *pfs_dev_uart_create (int uid, SERIAL_CONFIG *sc);

````

* __uid__ - UART Instance (0 or 1).
* __sc__ - Serial configuration
  + sc.baud - Baud rate
  + sc.parity - One of UART_PARITY_NONE , UART_PARITY_EVEN , UART_PARITY_ODD.
  + sc.data - Number of data bits (5 to 8)
  + sc.stop - Number of stop bits (1 or 2)
  + sc.tx - GPIO number for TX, or -1 for no TX pin
  + sc.rx - GPIO number for RX, or -1 for no RX pin
  + sc.cts - GPIO number for CTS, or -1 for no CTS pin
  + sc.rts - GPIO number for RTS, or -1 for no RTS pin

In CMake specify the __pfs_dev_uart__ link library for this device driver.

## IO Controls

Calls to IOCTL are uset to adjust the operation of an input / output
device.

````
int ioctl (int fd, long request, void *argp);
````

The routine returns zero on success, or -1 and sets errno on
failure.

As per Posix standard, the first parameter is a file handle, not
a `FILE *` pointer, use the following routine to obtain the file
handle:

````
int fileno (FILE *fp);
````

It should be noted that these IOCTLs are specific to PFS and have
no resemblance to Linux or Windows IOCTLs.

## ioctl (int fd, long IOC_RQ_MODE, int *mode)

Sets when a device read will return:

*  __IOC_MD_FULL__     - Only return when the buffer is full
*  __IOC_MD_NBLOCK__   - Never block waiting for input
*  __IOC_MD_ANY__      - Return as soon as at least one character input
*  __IOC_MD_CHR__      - Return when specified character received
*  __IOC_MD_CR__       - Return when Carrage Return character received
*  __IOC_MD_LF__       - Return when Line Feed character received
*  __IOC_MD_TLF__      - Replace terminating character by Line Feed
*  __IOC_MD_ECHO__     - Echo input characters to output

Applies to the Generic Input / Output driver and the UART driver.

## ioctl (int fd, long IOC_RQ_PURGE, NULL)

Purges all characters from the receive buffer

Applies to the Generic Input / Output driver and the UART driver.

## ioctl (int fd, long IOC_RQ_COUNT, int *count)

Gets the count of the number of characters in the receive buffer.

Applies to the Generic Input / Output driver and the UART driver.

## ioctl (int fd, long IOC_RQ_TOUT, int *tout)

Sets the read timeout in microseconds. A value of zero means
wait forever. To have zero timeout, set the mode __IOC_MD_NBLOCK__.

Applies to the Generic Input / Output driver and the UART driver.

## ioctl (int fd, long IOC_RQ_SCFG, SERIAL_CONFIG *sc)

Updates the baud rate (if non-zero), parity, number of data bits
and number of stop bits of a UART.

Applies to the UART driver only.

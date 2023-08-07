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

### USB Keyboard Driver

This uses the Pico in USB host mode, abd returns characters typed
on a USB keyboard attached to the USB port. 5V must be applied to
VSYS to power both the Pico and the Keyboard.

Typical initialisation is:

````
#include <pfs_dev_kbd.h>
#include <pfs_dev_keymap_uk.h>

    dev = pfs_dev_kbd_fetch (&pfs_dev_keymap_uk);
    pfs_mknod ("kbd", 0, dev);

````

See test/kbd_test.c for a complete example.

The keyboard may be used in one of two different modes:

* ASCII mode. Provides the ASCII code for each key typed. This
  produces no input for keys which don't have an ASCII code,
  such as the cursor keys (although you can define custom
  codes for these keys).

* Scan mode. Provides keyboard scan codes for each key press and
  key release event.

See below for more details. The initial mode is determined by
the parameter to `pfs_dev_kbd_fetch`. If a __keymap__ is
supplied (as shown) then the keyboard works in ASCII mode.
If the parameter is NULL, then the keyboard works in Scan Mode.

An IOCTL may be used to switch between modes once the keyboard
file is open.

Characters written to the keyboard device will update the keyboard
LEDs:

* Bit 0 controls the Num Lock LED
* Bit 1 controls the Caps Lock LED
* Bit 2 controls the Scroll Lock LED

To use this driver use the following CMake link libraries:

* pfs_dev_kbd
* pfs_dev_keymap_uk

#### ASCII Mode

The translation of key presses to ASCII is defined by a lookup
table with format given in __pfs_dev_keymap.h__:

````
typedef struct
    {
    int     nkey;
    struct
        {
        char    lower;
        char    upper;
        } key[];
    } PFS_DEV_KEYMAP;
````

A mapping for UK keyboards is given in __pfs_dev_keymap_uk.c__,
the start of which is:

````
const PFS_DEV_KEYMAP pfs_dev_keymap_uk =
    {
    0x68,                   // Number of key mappings
        {
        { 0x00, 0x00 },     // 0x00 - No key pressed
        { 0x00, 0x00 },     // 0x01 - Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
        { 0x00, 0x00 },     // 0x02 - Keyboard POST Fail
        { 0x00, 0x00 },     // 0x03 - Keyboard Error Undefined
        { 'a', 'A' },       // 0x04 - Keyboard a and A
        { 'b', 'B' },       // 0x05 - Keyboard b and B
        { 'c', 'C' },       // 0x06 - Keyboard c and C
        { 'd', 'D' },       // 0x07 - Keyboard d and D
        { 'e', 'E' },       // 0x08 - Keyboard e and E
````

The value of __nkey__ gives the highest scan code for which an
ASCII translation is defined. Any value up to 255 may be used
providing the following table has the corresponding number of
entries.

This is then followed by an ASCII code for each key without
(lower) or with (upper) the shift key. A value of zero indicates
no ASCII code for that key, so a press of that key will produce
no output.

By default many of the non-printing keys are defined as zero,
so produce no output. If the preprocessor variable
__EXTEND_ASCII__ is set to a non-zero value, then these keys
are given values with bit 7 set.

The Caps Lock, Scroll Lock and Num Lock keys will toggle the
state of the corresponding keyboard LEDs.

If the Caps Lock LED is lit, then lower case letters will be
converted to upper case.

If a Control key is pressed, then the 3 msb of the ASCII code
will be cleared. <Ctrl+@> will produce an ASCII code of zero.
It is not possibe to define a key that will produce an ASCII
without the Control key pressed.

#### Scan Mode

See pico-sdk/lib/tinyusb/src/class/hid/hid.h for a list of key scan
codes. In scan mode, the events are encoded in such a way that all
the more common keys are given as a single byte, with the remainder
as two bytes.

* keys with scan codes up to 0x57 are sent as the scan code, with
  bit 7 clear for key press events and bit 7 set for key release
  events.

* The modifier keys are sent as:
  + 0x58 - Left control key
  + 0x59 - Left shift key
  + 0x5A - Left ALT key
  + 0x5B - Left Window key
  + 0x5C - Right control key
  + 0x5D - Right shift key
  + 0x5E - Right ALT key
  + 0x5F - Right Window key
  Bit 7 clear for key press events and bit 7 set for key release
  events.

* The remaining keys (mainly keypad and special keys) are sent
  as two bytes:
  + First 0x70 plus the 4 msb of the key scan code.
  + Then 0x60 plus the 4 lsb of the key scan code.
  So Keypad 5 (scan code 0x5D) is sent as 0x75, 0x6D.
  Bit 7 of both bytes is clear for key press events and set on
  both bytes for release events.

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

## ioctl (int fd, long IOC_RQ_KEYMAP, PFS_DEV_KEYMAP *keymap)

Sets the ASCII key map to use. If NULL then selects Scan Mode.

Only applies to the USB keyboard driver.

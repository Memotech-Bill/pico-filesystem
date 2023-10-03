# pico-filesystem Test Cases

## pfs_test - General Test / Demo Program

The __pfs_test.c__ program serves two purposes:

* It confirms that all the components of pico-filesystem
  compile, link and execute as expected.

* It provides a simple example of the use of most of
  the features.

To build the program:

```bash
cd pico-filesystem/test
mkdir build
cd build
cmake -DPICO_BOARD=<board name> ..
make
```

Where `<board name>` is replaced by the name of the
Pico board being used.

To run the test with all the options selected (the
default) you need two terminal emulators, one
connected to the Pico default UART (connect this
one first), the other connected to the USB.

The test starts as soon as the USB connection is
established. On the USB terminal you should see:

```text
Terminal ready
Starting
Initialising file system
Writing File
Reading file
This was written by Memotech Bill's 'pico-filesystem'
Built on Aug  4 2023 20:52:26
Reading directory
  .
  dev
  sdcard
  pfs.txt
Listing devices
  .
  ..
  inout
  output
  uart1
  uart0
  tty0
Testing UART
```

At that point a message has been sent to the UART
terminal and is waiting a reply:

```text
Hello from Pico. What do you have to say ?
```

Type in a short reply, ending it with a RETURN. The characters
you type will not be displayed.

Once you have typed the RETURN key, the remainder of
the test, including whatever you typed, will be displayed
on the USB terminal.

```text
Received reply: Hi from Bill

Testing /dev/output
This string was written to /dev/output
Testing /dev/inout
Loading a string to read
This string was written to /dev/inout
Read from /dev/inout: A string loaded into /dev/inout

Finished
```

To run a test with only one terminal, disable the HAVE_UART
option.

## kbd_test - Keyboard Drver Test Program

The keyboard driver requires the Pico to be in USB host mode,
rather than client mode, so it cannot be included in pfs_test.

To run the kbd_test program, first load the program onto the
Pico, using BOOTSEL, __picotool__ or SWD. Then reconnect the
Pico with:

* A USB keyboard attached to the Pico USB connection.
* A 3.3v serial connection to the Pico default UART pins
  and a terminal emulator program controlling the serial
  connection. The serial connection should be set to
  115200 baud, 8 data bits, no parity, 1 stop bit, no flow
  control.
* A 5v power supply with +v to Pico VBUS pin and -ve to
  one of the GND pins. The power supply must be sufficient
  to power both the Pico and the keyboard.

Once powered up, the Pico will flash its onboard LED and
transmit stop characters (".") to indicate it is waiting.

In the terminal emulator, type a RETURN character to start
the test. You should then see:

```text
....Starting
Initialising file system
Opening keyboard
fp = 2000187C, fd = 3
ESC to switch between ASCII and Scan Codes
Start typing on USB keyboard ...
```

Characters typed on the USB keyboard will be displayed in
the terminal emulator. Most characters will be displayed
as-is. Control characters such as RETURN and BACKSPACE will
be displayed in hex. Note that keys such as the cursor keys
do not have any ASCII code and will therefore not display
anything.

Typing ESC on the USB keyboard will first display the hex
code for ESC (1B), and then switch to Scan Code mode. In
this mode the terminal emulator will display both key press
and key release events in hex.

Another ESC on the USB keyboard will revert to ASCII mode.

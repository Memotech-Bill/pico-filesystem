# pico-filesystem Test Case

The __pfs_test.c__ program serves two purposes:

* It confirms that all the components of pico-filesystem
  compile, link and execute as expected.

* It provides a simple example of the use of most of
  the features.

To build the program:

````
cd pico-filesystem/test
mkdir build
cd build
cmake -DPICO_BOARD=<board name> ..
make
````

Where `<board name>` is replaced by the name of the
Pico board being used.

To run the test with all the options selected (the
default) you need two terminal emulators, one
connected to the Pico default UART (connect this
one first), the other connected to the USB.

The test starts as soon as the USB connection is
established. On the USB terminal you should see:

````
Terminal ready
Starting
Initialising file sytsem
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
````

At that point a message has been sent to the UART
terminal and is waiting a reply:

````
Hello from Pico. What do you have to say ?
````

Type in a short reply, ending it with a RETURN. The characters
you type will not be displayed.

Once you have typed the RETURN key, the remainder of
the test, including whatever you typed, will be displayed
on the USB terminal.

````
Received reply: Hi from Bill

Testing /dev/output
This string was written to /dev/output
Testing /dev/inout
Loading a string to read
This string was written to /dev/inout
Read from /dev/inout: A string loaded into /dev/inout

Finished
````

To run a test with only one terminal, disable the HAVE_UART
option.

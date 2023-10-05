// pfs_test.c - Program to test pico-filesystem libraries
// Thanks to hippy for the original version: https://forums.raspberrypi.com/viewtopic.php?t=353918&start=25#p2124208

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <ioctl.h>
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pfs.h>
#include <pfs_dev_tty.h>
#include <pfs_dev_kbd.h>
#include <pfs_dev_keymap_uk.h>

int main (void)
    {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    stdio_init_all();
    int iLed = 0;
    while (true)
        {
        iLed = 1 - iLed;
	    gpio_put(PICO_DEFAULT_LED_PIN, iLed);
        printf (".");
        if ( getchar_timeout_us (500000) == 0x0D ) break;
        }
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // ----------------------------------------------------------------------
    
    printf("Starting\n");
    printf("Initialising file system\n");
    struct pfs_pfs *pfs = pfs_dev_fetch ();
        
    struct pfs_device *dev = pfs_dev_tty_fetch ();
    pfs_mknod ("tty0", 0, dev);

    dev = pfs_dev_kbd_fetch (&pfs_dev_keymap_uk);
    pfs_mknod ("kbd", 0, dev);
        
    pfs_mount(pfs, "/dev");
    
    // ----------------------------------------------------------------------

    printf ("Opening keyboard\n");
    FILE *fp = fopen ("/dev/kbd", "r");
    int fd = fileno (fp);
    printf ("fp = %p, fd = %d\n", fp, fd);
    printf ("ESC to switch between ASCII and Scan Codes\n"
        "Start typing on USB keyboard ...\n" );
    bool bScan = false;
    while (true)
        {
        int ch = fgetc (fp);
        if ( bScan )
            {
            printf (" %02X", ch);
            if ( ch == 0x29 )
                {
                printf ("\nASCII mode:\n");
                ioctl (fd, IOC_RQ_KEYMAP, &pfs_dev_keymap_uk);
                bScan = false;
                }
            }
        else
            {
            if (( ch >= 0x20 ) && ( ch < 0x7F )) printf ("%c", ch);
            else if ( ch == 0x0D ) printf (" 0D\n");
            else printf (" %02X ", ch);
            if ( ch == 0x1B )
                {
                printf ("\nScan Code mode:\n");
                ioctl (fd, IOC_RQ_KEYMAP, NULL);
                bScan = true;
                }
            }
        }
    }

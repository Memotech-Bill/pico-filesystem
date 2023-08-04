// pfs_test.c - Program to test pico-filesystem libraries
// Thanks to hippy for the original version: https://forums.raspberrypi.com/viewtopic.php?t=353918&start=25#p2124208

#include <stdio.h>
#include <dirent.h>
#include <pfs.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#if HAVE_LFS
#include <ffs_pico.h>
#endif
#if HAVE_DEV
#include <pfs_dev_tty.h>
#include <pfs_dev_uart.h>
#if HAVE_GIO
#include <pico/stdio.h>
#include <pfs_dev_gdd.h>
#include <pfs_dev_gio.h>
struct pfs_device *gio_dev;
#endif
#endif

#define BI_PFS_TAG              BINARY_INFO_MAKE_TAG('P', 'F')
#define BI_PFS_ID               0xAC95639A

bi_decl(bi_program_feature_group_with_flags(
        BI_PFS_TAG, BI_PFS_ID, "pfs",
        BI_NAMED_GROUP_SORT_ALPHA)
    );

bi_decl(bi_block_device(
        BI_PFS_TAG,
        "pico-filesystem",
        XIP_BASE + ROOT_OFFSET,
        ROOT_SIZE,
        NULL,
        BINARY_INFO_BLOCK_DEV_FLAG_READ |
        BINARY_INFO_BLOCK_DEV_FLAG_WRITE |
        BINARY_INFO_BLOCK_DEV_FLAG_PT_UNKNOWN)
    );

#if PICO_NO_FLASH

// I'm afraid I don't recall who the forum poster was who gets credit for this code

#include "pico/bootrom.h"
#include "hardware/resets.h"
#include "hardware/structs/ssi.h"

void InitXip(void)
    {
    if(ssi_hw->ssienr)
        {
        printf("  SSI: enabled, nothing to do\n");
        }
    else
        {
        printf("  SSI: disabled, enabling\n");

        uint32_t resets = RESETS_RESET_IO_QSPI_BITS | RESETS_RESET_PADS_QSPI_BITS;

        reset_block(resets);
        unreset_block_wait(resets);

        __compiler_memory_barrier();

        ((void(*)(void))rom_func_lookup_inline(ROM_FUNC_CONNECT_INTERNAL_FLASH))();
            ((void(*)(void))rom_func_lookup_inline(ROM_FUNC_FLASH_EXIT_XIP))();
            ((void(*)(void))rom_func_lookup_inline(ROM_FUNC_FLASH_ENTER_CMD_XIP))();

            ssi_hw->ssienr = 0;
            ssi_hw->baudr = 16;
            ssi_hw->ssienr = 1;

            ((void(*)(void))rom_func_lookup_inline(ROM_FUNC_FLASH_FLUSH_CACHE))();

            __compiler_memory_barrier();

            printf("  SSI: enabled\n");
        }
    }

#endif

void sc_cfg (int uid, SERIAL_CONFIG *psc)
    {
    psc->baud = 115200;
    psc->parity = UART_PARITY_NONE;
    psc->data = 8;
    psc->stop = 1;
    if ( uid == PICO_DEFAULT_UART )
        {
        psc->tx = PICO_DEFAULT_UART_TX_PIN;
        psc->rx = PICO_DEFAULT_UART_RX_PIN;
        }
    else if ( uid == 0 )
        {
        psc->tx = 0;
        psc->rx = 1;
        }
    else
        {
        psc->tx = 4;
        psc->rx = 5;
        }
    psc->cts = -1;
    psc->rts = -1;
    }

void echo_char (char ch)
    {
    printf ("%c", ch);
    }

int main (void)
    {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    stdio_init_all();
    int iLed = 0;
    while (!stdio_usb_connected())
        {
        iLed = 1 - iLed;
	    gpio_put(PICO_DEFAULT_LED_PIN, iLed);
        sleep_ms(500);
        }
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    printf("Starting\n");
    // ----------------------------------------------------------------------
#if PICO_NO_FLASH
    printf("Initialising Flash access\n");
    InitXip();
#endif
    // ----------------------------------------------------------------------
    printf("Initialising file sytsem\n");
        {
        struct pfs_pfs *pfs;
#if HAVE_LFS
        struct lfs_config cfg;
        ffs_pico_createcfg(&cfg, ROOT_OFFSET, ROOT_SIZE);
        pfs = pfs_ffs_create(&cfg);
        pfs_mount(pfs, "/");
#if HAVE_FAT
        pfs = pfs_fat_create();
        pfs_mount(pfs, "/sdcard");
#endif
#else
#if HAVE_FAT
        pfs = pfs_fat_create();
        pfs_mount(pfs, "/");
#endif
#endif
#if HAVE_DEV
        pfs = pfs_dev_fetch ();
        struct pfs_device *dev = pfs_dev_tty_fetch ();
        pfs_mknod ("tty0", 0, dev);

        SERIAL_CONFIG sc;
        sc_cfg (0, &sc);
        dev = pfs_dev_uart_create (0, &sc);
        if ( dev != NULL ) pfs_mknod ("uart0", 0, dev);

        sc_cfg (1, &sc);
        dev = pfs_dev_uart_create (1, &sc);
        if ( dev != NULL ) pfs_mknod ("uart1", 0, dev);

#if HAVE_GIO
        dev = pfs_dev_gdd_create (echo_char);
        if ( dev != NULL ) pfs_mknod ("output", 0, dev);

        gio_dev = pfs_dev_gio_create (echo_char, 64, GIO_M_CR | GIO_M_TLF);
        if ( gio_dev != NULL ) pfs_mknod ("inout", 0, gio_dev);
#endif
        pfs_mount(pfs, "/dev");
#endif
        }
    // ----------------------------------------------------------------------
    printf("Writing File\n");
        {
        FILE * fp = fopen("pfs.txt", "w");
        fprintf(fp, "This was written by Memotech Bill's 'pico-filesystem'\n");
#if PICO_NO_FLASH
        fprintf(fp, "Running in RAM\n");
#endif
        fprintf(fp, "Built on %s %s\n", __DATE__, __TIME__);
        fclose(fp);
        }
    // ----------------------------------------------------------------------
    printf("Reading file\n");
        {
        FILE * fp = fopen("pfs.txt", "r");
        int c;
        while ((c = getc(fp)) != EOF) {
            printf("%c", c);
            }
        fclose(fp);
        }
    // ----------------------------------------------------------------------
    printf("Reading directory\n");
        {
        DIR * dp = opendir("/");
        struct dirent *ep;
        while ((ep = readdir(dp)) != NULL) {
            printf("  %s\n", ep->d_name);
            }
        closedir(dp);
        }
    // ----------------------------------------------------------------------
#if HAVE_DEV
    printf("Listing devices\n");
        {
        DIR * dp = opendir("/dev");
        struct dirent *ep;
        while ((ep = readdir(dp)) != NULL) {
            printf("  %s\n", ep->d_name);
            }
        closedir(dp);
        }
    // ----------------------------------------------------------------------
    printf("Testing UART\n");
        {
#if PICO_DEFAULT_UART == 0
        FILE *fp = fopen ("/dev/uart0", "r+");
#else
        FILE *fp = fopen ("/dev/uart1", "r+");
#endif
        fprintf (fp, "Hello from Pico. What do you have to say ?\r\n");
        char reply[512];
        fgets (reply, sizeof (reply), fp);
        printf ("Received reply: %s\n", reply);
        fclose (fp);
        }
#if HAVE_GIO
    printf ("Testing /dev/output\n");
        {
        FILE *fp = fopen ("/dev/output", "w");
        fprintf (fp, "This string was written to /dev/output\n");
        fclose (fp);
        }
    printf ("Testing /dev/inout\n");
        {
        FILE *fp = fopen ("/dev/inout", "w+");
        fprintf (fp, "This string was written to /dev/inout\n");
        printf ("Loading a string to read\n");
        const char *psMsg = "A string loaded into /dev/inout\r";
        while (*psMsg != '\0')
            {
            pfs_dev_gio_input (gio_dev, *psMsg);
            ++psMsg;
            }
        char reply[512];
        memset (reply, 0, sizeof (reply));
        fgets (reply, sizeof (reply), fp);
        printf ("Read from /dev/inout: %s\n", reply);
        fclose (fp);
        }
#endif    
#endif
    // ----------------------------------------------------------------------
    printf("Finished\n");
    while (true)
        {
        sleep_ms(1000);
        }
    }

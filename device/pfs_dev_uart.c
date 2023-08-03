// pfs_dev_uart.c - A PFS character device for Pico UARTs
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>
#include <sys/errno.h>
#include <pico/critical_section.h>
#include <hardware/uart.h>
#include <hardware/structs/uart.h>
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <sconfig.h>
#include <pfs_private.h>

#include <stdio.h>

#ifndef STATIC
#define STATIC  static
#endif

STATIC struct pfs_file *uart_open (const struct pfs_device *dev, const char *name, int oflags);
STATIC int uart_read (struct pfs_file *fd, char *buffer, int length);
STATIC int uart_write (struct pfs_file *fd, char *buffer, int length);

#define NDATA   512     // Length of serial receive buffer (must be a power of 2)

STATIC struct pfs_dev_uart
    {
    struct pfs_file *   (*open)(const struct pfs_device *dev, const char *name, int oflags);
    uart_inst_t *       uart;
    critical_section_t  ucs;
    int                 rptr;
    int                 wptr;
    char                data[NDATA];
    } *uart_dev[NUM_UARTS] = {NULL, NULL};

STATIC const struct pfs_v_file uart_v_file =
    {
    NULL,           // close
    uart_read,      // read
    uart_write,     // write
    NULL,           // lseek
    NULL,           // fstat
    NULL,           // isatty
    NULL,           // ioctl
    };

STATIC void uart_input (struct pfs_dev_uart *pud)
    {
    critical_section_enter_blocking (&pud->ucs);
    int wend = ( pud->rptr - 1 ) & ( NDATA - 1 );
    while ((pud->wptr != wend) && (uart_is_readable (pud->uart)))
        {
        pud->data[pud->wptr] = uart_getc (pud->uart);
        // uart_putc_raw (pud->uart, pud->data[pud->wptr]);
        pud->wptr = (++pud->wptr) & ( NDATA - 1 );
        }
    if ( pud->wptr == wend )
        {
        hw_clear_bits (&((uart_hw_t *)pud->uart)->cr, UART_UARTCR_RTS_BITS);
        hw_clear_bits (&((uart_hw_t *)pud->uart)->imsc, UART_UARTIMSC_RXIM_BITS);
        }
    critical_section_exit (&pud->ucs);
    }

STATIC void irq_uart0 (void)
    {
    if ( uart_dev[0] != NULL ) uart_input (uart_dev[0]);
    }

STATIC void irq_uart1 (void)
    {
    if ( uart_dev[1] != NULL ) uart_input (uart_dev[1]);
    }

STATIC int uart_read (struct pfs_file *fd, char *buffer, int length)
    {
    char *bptr = buffer;
    struct pfs_dev_uart *pud = (struct pfs_dev_uart *) fd->pfs;
    int nread = 0;
    uart_input (pud);
    while (length > 0)
        {
        while ( pud->rptr == pud->wptr )
            {
            __wfi ();
            uart_input (pud);
            }
        *bptr = pud->data[pud->rptr];
        pud->rptr = (++pud->rptr) & ( NDATA - 1 );
        ++nread;
        --length;
        if ( *bptr == '\r' )
            {
            *bptr = '\n';
            break;
            }
        ++bptr;
        }
    critical_section_enter_blocking (&pud->ucs);
    hw_set_bits (&((uart_hw_t *)pud->uart)->cr, UART_UARTCR_RTS_BITS);
    hw_set_bits (&((uart_hw_t *)pud->uart)->imsc, UART_UARTIMSC_RXIM_BITS);
    critical_section_exit (&pud->ucs);
    return nread;
    }

STATIC int uart_write (struct pfs_file *fd, char *buffer, int length)
    {
    struct pfs_dev_uart *pud = (struct pfs_dev_uart *) fd->pfs;
    uart_write_blocking (pud->uart, buffer, length);
    return length;
    }

STATIC bool uart_pin_valid (int uid, int func, int pin)
    {
    if (( pin < 0 ) || ( pin > 29 )) return false;
    pin -= func;
    if ( uid == 0 )
        {
        if (( pin == 0 ) || ( pin == 12 ) || ( pin == 16 ) || ( pin == 28 )) return true;
        }
    else if ( uid == 1 )
        {
        if (( pin == 4 ) || ( pin == 8 ) || ( pin == 20 ) || ( pin == 24 )) return true;
        }
    return false;
    }

STATIC bool uopen (int uid, SERIAL_CONFIG *sc)
    {
    struct pfs_dev_uart *pud = uart_dev[uid];
    pud->uart = uart_get_instance (uid);
    pud->rptr = 0;
    pud->wptr = 0;
    critical_section_init (&pud->ucs);
    if ( uart_init (pud->uart, sc->baud) == 0 ) return false;
    if ( sc->tx >= 0 )
        {
        if ( ! uart_pin_valid (uid, 0, sc->tx) ) return false;
        gpio_set_function (sc->tx, GPIO_FUNC_UART);
        }
    if ( sc->rx >= 0 )
        {
        if ( ! uart_pin_valid (uid, 1, sc->rx) ) return false;
        gpio_set_function (sc->rx, GPIO_FUNC_UART);
        }
    if ( sc->cts >= 0 )
        {
        if ( ! uart_pin_valid (uid, 2, sc->cts) ) return false;
        gpio_set_function (sc->cts, GPIO_FUNC_UART);
        uart_set_hw_flow (pud->uart, true, false);
        }
    else
        {
        uart_set_hw_flow (pud->uart, false, false);
        }
    if ( sc->rts >= 0 )
        {
        if ( ! uart_pin_valid (uid, 3, sc->rts) ) return false;
        gpio_set_function (sc->rts, GPIO_FUNC_UART);
        }
    if ( uid == 0 )
        {
        irq_set_exclusive_handler(UART0_IRQ, irq_uart0);
        irq_set_enabled(UART0_IRQ, true);
        }
    else
        {
        irq_set_exclusive_handler(UART1_IRQ, irq_uart1);
        irq_set_enabled(UART1_IRQ, true);
        }
    uart_set_irq_enables(pud->uart, true, false);
    hw_clear_bits (&((uart_hw_t *)pud->uart)->ifls, UART_UARTIFLS_RXIFLSEL_BITS);
    hw_set_bits (&((uart_hw_t *)pud->uart)->cr, UART_UARTCR_RTS_BITS);
    hw_set_bits (&((uart_hw_t *)pud->uart)->imsc, UART_UARTIMSC_RXIM_BITS);
    if (( sc->data < 5 ) || ( sc->data > 8 )) return false;
    if (( sc->stop < 1 ) || ( sc->stop > 2 )) return false;
    int iPar;
    if (( sc->parity != UART_PARITY_NONE ) && ( sc->parity != UART_PARITY_EVEN )
        && ( sc->parity != UART_PARITY_ODD )) return false;
    uart_set_format (pud->uart, sc->data, sc->stop, sc->parity);
    uart_set_irq_enables (pud->uart, true, false);
    }

void uclose (int uid)
    {
    if (( uid < 0 ) || ( uid > 1 )) return;
    struct pfs_dev_uart *pud = uart_dev[uid];
    uart_deinit (pud->uart);
    critical_section_deinit (&pud->ucs);
    }

STATIC struct pfs_file *uart_open (const struct pfs_device *dev, const char *name, int oflags)
    {
    struct pfs_file *uart = (struct pfs_file *) malloc (sizeof (struct pfs_file));
    if ( uart == NULL )
        {
        pfs_error (ENOMEM);
        return NULL;
        }
    uart->entry = &uart_v_file;
    uart->pfs = (struct pfs_pfs *) dev;
    uart->pn = NULL;
    return uart;
    }

const struct pfs_device *pfs_dev_uart_create (int uid, SERIAL_CONFIG *sc)
    {
    if (( uid < 0 ) || ( uid > 1 )) return NULL;
    if ( uart_dev[uid] != NULL )
        {
        uopen (uid, sc);
        return (const struct pfs_device *) uart_dev[uid];
        }
    uart_dev[uid] = (struct pfs_dev_uart *) malloc (sizeof (struct pfs_dev_uart));
    if ( uart_dev[uid] == NULL ) return NULL;
    uart_dev[uid]->open = uart_open;
    if ( uopen (uid, sc) ) return (const struct pfs_device *) uart_dev[uid];
    uclose (uid);
    free (uart_dev[uid]);
    uart_dev[uid] = NULL;
    return NULL;
    }

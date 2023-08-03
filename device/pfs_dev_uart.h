// pfs_dev_uart.h - UART device for pico-filesystem

#ifndef PFS_DEV_UART_H
#define PFS_DEV_UART_H

#include <pfs.h>
#include <sconfig.h>

const struct pfs_device *pfs_dev_uart_create (int uid, SERIAL_CONFIG *sc);

#endif

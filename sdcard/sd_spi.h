/* sp_sdi.h - SPI routines to access SD card */
// Copyright (c) 2023, Memotech-Bill
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SD_SPI_H
#define SD_SPI_H

#include <stdint.h>

typedef enum {sdtpUnk, sdtpVer1, sdtpVer2, sdtpHigh} SD_TYPE;
extern SD_TYPE sd_type;

bool sd_spi_init (void);
void sd_spi_term (void);
bool sd_spi_read (uint lba, uint8_t *buff);
bool sd_spi_write (uint lba, const uint8_t *buff);

#endif

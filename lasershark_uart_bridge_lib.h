/*
lasershark_uart_bridge_lib.h - Library to help hosts communicate with 
devices attached to a Lasershark board's UART.
Copyright (C) 2012 Jeffrey Nelson <nelsonjm@macpod.net>

This file is part of Lasershark's USB Host App.

Lasershark is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Lasershark is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Lasershark. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LASERSHARK_UB_LIB_H_
#define LASERSHARK_UB_LIB_H_
#include <libusb.h>
#include <stdint.h>


#define LASERSHARK_UB_CMD_SUCCESS 0x00
#define LASERSHARK_UB_CMD_FAIL 0x01
#define LASERSHARK_UB_CMD_UNKNOWN 0xFF

#define LASERSHARK_UB_VERSION 1

#define LASERSHARK_UB_CMD_TX 0xA0
#define LASERSHARK_UB_CMD_RX 0xA1
#define LASERSHARK_UB_CMD_RX_READY 0xA2
#define LASERSHARK_UB_CMD_MAX_TX 0xA3
#define LASERSHARK_UB_CMD_MAX_RX 0xA4
#define LASERSHARK_UB_CMD_CLEAR_RX_FIFO 0xA5
#define LASERSHARK_UB_CMD_VERSION 0XA6

#define LASERSHARK_UB_RX_NOT_READY 0x0
#define LASERSHARK_UB_RX_READY 0x1


// Get UB version
int lasershark_ub_get_version(libusb_device_handle *devh_ctl, uint32_t *version);

// Get max number of bytes that can possibly be accepted at one time.
int lasershark_ub_get_max_rx(libusb_device_handle *devh_ub, uint8_t *len);

// Get max number of bytes that can possibly be received at one time.
int lasershark_ub_get_max_tx(libusb_device_handle *devh_ub, uint8_t *len);

// Clears out any bytes in the UB.
int lasershark_ub_clear_rx_fifo(libusb_device_handle *dev_ub);

int lasershark_ub_tx(libusb_device_handle *dev_ub, uint8_t len, uint8_t *buf, uint8_t *len_txed);

int lasershark_ub_rx(libusb_device_handle *dev_ub, uint8_t len, uint8_t *buf, uint8_t *len_rxed);

#endif /* LASERSHARK_3D_PRINTER_LIB_H_ */

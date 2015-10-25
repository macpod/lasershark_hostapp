/*
lasershark_uart_bridge_lib.c - Library to help hosts communicate with 
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

#include "lasershark_uart_bridge_lib.h"
#include <stdint.h>
#include <libusb.h>
#include <string.h>


// Helper function
int lasershark_ub_get_uint8(libusb_device_handle *devh_ub, uint8_t command, uint8_t *val)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

    data[0] = command;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_UB_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);
    if(r != 0 || actual != 64 || data[1] != LASERSHARK_UB_CMD_SUCCESS) {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_UB_CMD_FAIL;
    }
    *val = data[2];

    return LASERSHARK_UB_CMD_SUCCESS;
}


// Helper function
int lasershark_ub_get_uint32(libusb_device_handle *devh_ub, uint8_t command, uint32_t *val)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

    data[0] = command;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_UB_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_UB_CMD_SUCCESS) {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_UB_CMD_FAIL;
    }
    memcpy(val, data + 2, sizeof(uint32_t));

    return LASERSHARK_UB_CMD_SUCCESS;
}


int lasershark_ub_get_version(libusb_device_handle *devh_ub, uint32_t *version)
{
    return lasershark_ub_get_uint32(devh_ub, LASERSHARK_UB_CMD_VERSION, version);
}


int lasershark_ub_get_max_rx(libusb_device_handle *devh_ub, uint8_t *max_rx)
{
    return lasershark_ub_get_uint8(devh_ub, LASERSHARK_UB_CMD_MAX_RX, max_rx);
}


int lasershark_ub_get_max_tx(libusb_device_handle *devh_ub, uint8_t *max_tx)
{
    return lasershark_ub_get_uint8(devh_ub, LASERSHARK_UB_CMD_MAX_TX, max_tx);
}


int lasershark_ub_clear_rx_fifo(libusb_device_handle *devh_ub)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

    data[0] = LASERSHARK_UB_CMD_CLEAR_RX_FIFO;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_UB_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_UB_CMD_SUCCESS) {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_UB_CMD_FAIL;
    }

    return LASERSHARK_UB_CMD_SUCCESS;
}


int lasershark_ub_tx(libusb_device_handle *devh_ub, uint8_t tx_len, uint8_t *buf, uint8_t *len_txed)
{
    unsigned char data[64];
    int r, actual;
    int len = 2+tx_len;

    if (len > 64) {
        return LASERSHARK_UB_CMD_FAIL;
    }

    data[0] = LASERSHARK_UB_CMD_TX;
    data[1] = tx_len;
    memcpy(data+2, buf, tx_len);

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_UB_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_UB_CMD_SUCCESS) {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_UB_CMD_FAIL;
    }
    memcpy(len_txed, data + 2, sizeof(uint8_t));

    return LASERSHARK_UB_CMD_SUCCESS;
}


int lasershark_ub_rx(libusb_device_handle *devh_ub, uint8_t rx_len, uint8_t *buf, uint8_t *len_rxed)
{
    unsigned char data[64];
    int r, actual;
    int len = 2;

    data[0] = LASERSHARK_UB_CMD_RX;
    data[1] = rx_len;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_UB_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ub, (2 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_UB_CMD_SUCCESS) {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_UB_CMD_FAIL;
    }

    len = data[2];
    if (len > 61) {
        return LASERSHARK_UB_CMD_FAIL;
    }
    memcpy(buf, data + 3, len);
    *len_rxed = len;

    return LASERSHARK_UB_CMD_SUCCESS;
}


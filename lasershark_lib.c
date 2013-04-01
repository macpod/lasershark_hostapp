/*
lasershark_lib.c - Library to help host apps talk with Lasershark devices.
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

#include "lasershark_lib.h"

#include <stdint.h>
#include <libusb.h>
#include <string.h>
#include <libusb.h>


// Helper function
static int get_uint8(libusb_device_handle *devh_ctl, uint8_t command, uint8_t *val)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

    data[0] = command;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);
    if(r != 0 || actual != 64 || data[1] != LASERSHARK_CMD_SUCCESS)
    {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_CMD_FAIL;
    }
    *val = data[2];

    return LASERSHARK_CMD_SUCCESS;
}


// Helper function
static int get_uint32(libusb_device_handle *devh_ctl, uint8_t command, uint32_t *val)
{
    unsigned char data[64];
    int r, actual;
    int len = 1;

    data[0] = command;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_CMD_SUCCESS)
    {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_CMD_FAIL;
    }
    memcpy(val, data + 2, sizeof(uint32_t));

    return LASERSHARK_CMD_SUCCESS;
}


static int set_uint8(libusb_device_handle *devh_ctl, uint8_t command, uint8_t val)
{
    unsigned char data[64];
    int r, actual;
    int len = 2;

    data[0] = command;
    data[1] = val;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);
    if(r != 0 || actual != 64 || data[1] != LASERSHARK_CMD_SUCCESS)
    {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_CMD_FAIL;
    }

    return LASERSHARK_CMD_SUCCESS;
}

static int set_uint32(libusb_device_handle *devh_ctl, uint8_t command, uint32_t val)
{
    unsigned char data[64];
    int r, actual;
    int len = 1 + sizeof(uint32_t);

    data[0] = command;
    memcpy(data + 1, &val, sizeof(uint32_t));

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_OUT), data, len, &actual, 0);
    if(r != 0 || actual != len)
        return LASERSHARK_CMD_FAIL;

    r = libusb_bulk_transfer(devh_ctl, (1 | LIBUSB_ENDPOINT_IN), data, 64, &actual, 0);

    if(r != 0 || actual != 64 || data[1] != LASERSHARK_CMD_SUCCESS)
    {
        //printf("Read Error: %d, %d\n",  r, actual);
        return LASERSHARK_CMD_FAIL;
    }

    return LASERSHARK_CMD_SUCCESS;
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int set_output(libusb_device_handle *devh_ctl, uint8_t state)
{
    return set_uint8(devh_ctl, LASERSHARK_CMD_SET_OUTPUT, state);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_output(libusb_device_handle *devh_ctl, uint8_t *state)
{
    return get_uint8(devh_ctl, LASERSHARK_CMD_GET_OUTPUT, state);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int set_ilda_rate(libusb_device_handle *devh_ctl, uint32_t rate)
{
    return set_uint32(devh_ctl, LASERSHARK_CMD_SET_ILDA_RATE, rate);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_ilda_rate(libusb_device_handle *devh_ctl, uint32_t *rate)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_ILDA_RATE, rate);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_max_ilda_rate(libusb_device_handle *devh_ctl, uint32_t *rate)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_MAX_ILDA_RATE, rate);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_samp_element_count(libusb_device_handle *devh_ctl, uint32_t *samp_element_count)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_SAMP_ELEMENT_COUNT, samp_element_count);
}


// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_packet_sample_count(libusb_device_handle *devh_ctl, uint32_t *packet_sample_count)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_PACKET_SAMP_COUNT, packet_sample_count);
}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_dac_min(libusb_device_handle *devh_ctl, uint32_t *dac_min)
{

    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_DAC_MIN, dac_min);
}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_dac_max(libusb_device_handle *devh_ctl, uint32_t *dac_max)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_DAC_MAX, dac_max);

}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_ringbuffer_sample_count(libusb_device_handle *devh_ctl, uint32_t *ringbuffer_sample_count)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_RINGBUFFER_SAMPLE_COUNT, ringbuffer_sample_count);
}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_ringbuffer_empty_sample_count(libusb_device_handle *devh_ctl, uint32_t *ringbuffer_empty_sample_count)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT, ringbuffer_empty_sample_count);
}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_fw_major_version(libusb_device_handle *devh_ctl, uint32_t *fw_version_major)
{
    return get_uint32(devh_ctl, LASERSHARK_CMD_GET_LASERSHARK_FW_MAJOR_VERSION, fw_version_major);
}

// Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
int get_fw_minor_version(libusb_device_handle *devh_ctl, uint32_t *fw_version_minor)
{
    return get_uint32(devh_ctl, LASERSHARK_GMD_GET_LASERSHARK_FW_MINOR_VERSION, fw_version_minor);
}


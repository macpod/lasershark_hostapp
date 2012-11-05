/*
lasershark_lib.h - Library to help host apps talk with Lasershark devices.
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

#ifndef LASERSHARK_LIB_H_
#define LASERSHARK_LIB_H_

#include <stdint.h>
#include <libusb.h>


#define LASERSHARK_CMD_SUCCESS 0x00
#define LASERSHARK_CMD_FAIL 0x01
#define LASERSHARK_CMD_UNKNOWN 0xFF

// Set/Get output commands
#define LASERSHARK_CMD_SET_OUTPUT 0x80
#define LASERSHARK_CMD_GET_OUTPUT 0x81
#define LASERSHARK_CMD_OUTPUT_ENABLE 0x01
#define LASERSHARK_CMD_OUTPUT_DISABLE 0x00

// Set/get current ilda rate
#define LASERSHARK_CMD_SET_ILDA_RATE 0x82
#define LASERSHARK_CMD_GET_ILDA_RATE 0x83

// Get max ilda rate
#define LASERSHARK_CMD_GET_MAX_ILDA_RATE 0X84

// Get number of ilda elements per sample
#define LASERSHARK_CMD_GET_SAMP_ELEMENT_COUNT 0X85
// Get number of ilda samples that can be transmitter per packet.
#define LASERSHARK_CMD_GET_PACKET_SAMP_COUNT 0x86

// Get min dac value
#define LASERSHARK_CMD_GET_DAC_MIN 0x87
// Get max dac value
#define LASERSHARK_CMD_GET_DAC_MAX 0x88

/*
Packet format
To device
[1byte command][0-xbyte data...]

To computer:
[1byte command][1byte status][0-xbyte data...]

*/



int set_output(libusb_device_handle *devh_ctl, uint8_t state);

int get_output(libusb_device_handle *devh_ctl, uint8_t *state);

int set_ilda_rate(libusb_device_handle *devh_ctl, uint32_t rate);

int get_ilda_rate(libusb_device_handle *devh_ctl, uint32_t *rate);

int get_max_ilda_rate(libusb_device_handle *devh_ctl, uint32_t *rate);

int get_samp_element_count(libusb_device_handle *devh_ctl, uint32_t *packet_sample_count);

int get_packet_sample_count(libusb_device_handle *devh_ctl, uint32_t *packet_sample_count);

int get_dac_min(libusb_device_handle *devh_ctl, uint32_t *dac_min);

int get_dac_max(libusb_device_handle *devh_ctl, uint32_t *dac_max);

#endif /* LASERSHARK_LIB_H_ */


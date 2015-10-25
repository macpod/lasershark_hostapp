/*
ls_ub_twostep_lib.c - Allows communications to Twostep board via
a Lasershark board's UART.
Copyright (C) 2013 Jeffrey Nelson <nelsonjm@macpod.net>

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

#ifndef LS_UB_TWOSTEP_LIB_H_
#define LS_UB_TWOSTEP_LIB_H_
#include <stdint.h>
#include <stdbool.h>
#include <libusb.h>
#include "twostep_common_lib.h"


#define LS_UB_TWOSTEP_SUCCESS 0x00
// Occurs if lasershark's uart bridge had an issue
#define LS_UB_TWOSTEP_UB_FAIL 0x01
// Occurs if lasershark's uart bridge worked, but didn't tx all info
#define LS_UB_TWOSTEP_TX_FAIL 0x02
// Occurs if lasershark's uart bridge worked, but didn't rx all info
#define LS_UB_TWOSTEP_RX_FAIL 0x03
// Occurs if issue was on twostep's side.
#define LS_UB_TWOSTEP_TS_PROTO_FAIL 0x04
// Occurs if twostep cmd/resp was valid.. but user attempted action failed.
#define LS_UB_TWOSTEP_TS_CMD_FAIL 0x05


int ls_ub_twostep_set_steps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint32_t steps);
int ls_ub_twostep_set_safe_steps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint32_t steps);
int ls_ub_twostep_set_step_until_switch(libusb_device_handle *devh_ub, uint8_t stepper_num);

int ls_ub_twostep_start(libusb_device_handle *devh_ub, uint8_t stepper_bitfield);
int ls_ub_twostep_stop(libusb_device_handle *devh_ub, uint8_t stepper_bitfield);

int ls_ub_twostep_get_is_moving(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *stepper_moving);

int ls_ub_twostep_set_enable(libusb_device_handle *devh_ub, uint8_t stepper_num, bool enable);
int ls_ub_twostep_get_enable(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *enable);

int ls_ub_twostep_set_microsteps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint8_t microstep_bitfield);
int ls_ub_twostep_get_microsteps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint8_t *microstep_bitfield);

int ls_ub_twostep_set_dir(libusb_device_handle *devh_ub, uint8_t stepper_num, bool high);
int ls_ub_twostep_get_dir(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *high);

uint8_t ls_ub_twostep_set_current(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t current);
uint8_t ls_ub_twostep_get_current(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t *current);

uint8_t ls_ub_twostep_set_100uS_delay(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t delay);
uint8_t ls_ub_twostep_get_100uS_delay(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t *delay);

uint8_t ls_ub_twostep_get_switch_status(libusb_device_handle *devh_ub, uint8_t *switch_bitfield);

uint8_t ls_ub_twostep_get_version(libusb_device_handle *devh_ub, uint8_t *version);


#endif


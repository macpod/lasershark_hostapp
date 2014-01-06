/*
twostep_host_lib.h - Contains functions that assist with 
host sided communications between a host and twostep devices.
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

#ifndef TWOSTEP_HOST_LIB_H_
#define TWOSTEP_HOST_LIB_H_
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "twostep_common_lib.h"


// Expects resp to be valid.
bool twostep_resp_indicates_passed(uint8_t *resp_buf, uint8_t len);


// These expect the buffer to be of at least proper len. If you can stuff it in, it doesn't care about the args.
// They all return the size of the cmd buf.
uint8_t twostep_cmd_set_steps(uint8_t *buf, uint8_t stepper_num, uint32_t steps);
uint8_t twostep_cmd_set_safe_steps(uint8_t *buf, uint8_t stepper_num, uint32_t steps);
uint8_t twostep_cmd_set_step_until_switch(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_start(uint8_t *buf, uint8_t stepper_bitfield);
uint8_t twostep_cmd_stop(uint8_t *buf, uint8_t stepper_bitfield);

uint8_t twostep_cmd_get_is_moving(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_set_enable(uint8_t *buf, uint8_t stepper_num, bool enable);
uint8_t twostep_cmd_get_enable(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_set_microsteps(uint8_t *buf, uint8_t stepper_num, uint8_t microstep_bitfield);
uint8_t twostep_cmd_get_microsteps(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_set_dir(uint8_t *buf, uint8_t stepper_num, bool high);
uint8_t twostep_cmd_get_dir(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_set_current(uint8_t *buf, uint8_t stepper_num, uint16_t current);
uint8_t twostep_cmd_get_current(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_set_100uS_delay(uint8_t *buf, uint8_t stepper_num, uint16_t delay);
uint8_t twostep_cmd_get_100uS_delay(uint8_t *buf, uint8_t stepper_num);

uint8_t twostep_cmd_get_switch_status(uint8_t *buf);

uint8_t twostep_cmd_get_version(uint8_t *buf);


// These expect the buffer to be of at least proper len and expects the response format to at least be valid.
// Returns false if cmd isn't what is expected or received params are incorrect.
bool twostep_resp_get_is_moving(uint8_t *buf, uint8_t *is_moving);
bool twostep_resp_get_enable(uint8_t *buf,uint8_t *enable);
bool twostep_resp_get_microsteps(uint8_t *buf,uint8_t *microstep_bitfield);
bool twostep_resp_get_dir(uint8_t *buf,uint8_t *high);
bool twostep_resp_get_current(uint8_t *buf,uint16_t *current);
bool twostep_resp_get_100uS_delay(uint8_t *buf, uint16_t *delay);
bool twostep_resp_get_switch_status(uint8_t *buf, uint8_t *switches);
bool twostep_resp_get_version(uint8_t *buf, uint8_t *version);

#endif


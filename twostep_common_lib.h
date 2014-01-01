/*
twostep_common_lib.h - Contains constants and functions that
assist with communications between a host and twostep devices.
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

#ifndef TWOSTEP_COMMON_LIB_H_
#define TWOSTEP_COMMON_LIB_H_
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>


#define TWOSTEP_VERSION 0x01


#define TWOSTEP_CMD_SUCCESS 0x00
#define TWOSTEP_CMD_FAIL 0x01
#define TWOSTEP_CMD_UNKNOWN 0xff


#define TWOSTEP_BUF_SIZE 16


#define TWOSTEP_START_TOKEN '='
#define TWOSTEP_END1_TOKEN '\r'
#define TWOSTEP_END2_TOKEN '\n'


#define TWOSTEP_SET_STEPS 0x10
//stepper_set_steps(uint8_t stepper_num, uint32_t steps);
#define TWOSTEP_SET_STEPS_CMD_LEN 9
#define TWOSTEP_SET_STEPS_RESP_LEN 5

#define TWOSTEP_SET_SAFE_STEPS 0x11
//stepper_set_safe_steps(uint8_t stepper_number, uint32_t steps);
#define TWOSTEP_SET_SAFE_STEPS_CMD_LEN 9
#define TWOSTEP_SET_SAFE_STEPS_RESP_LEN 5

#define TWOSTEP_SET_STEP_UNTIL_RELAY 0x12
// stepper_set_step_until_relay(uint8_t stepper_num);
#define TWOSTEP_SET_STEP_UNTIL_RELAY_CMD_LEN 5
#define TWOSTEP_SET_STEP_UNTIL_RELAY_RESP_LEN 5

#define TWOSTEP_START 0x13
// stepper_start(uint8_t stepper_bitfield);
#define TWOSTEP_START_CMD_LEN 5
#define TWOSTEP_START_RESP_LEN 5

#define TWOSTEP_STOP 0x14
// stepper_stop(uint8_t stepper_bitfield);
#define TWOSTEP_STOP_CMD_LEN 5
#define TWOSTEP_STOP_RESP_LEN 5

#define TWOSTEP_GET_IS_MOVING 0x15
// stepper_get_moving(uint8_t stepper_num, uint8_t is_moving);
#define TWOSTEP_GET_IS_MOVING_CMD_LEN 5
#define TWOSTEP_GET_IS_MOVING_RESP_LEN 6

#define TWOSTEP_SET_ENABLE 0x16
//  stepper_set_enable(uint8_t stepper_num, uint8_t enable);
#define TWOSTEP_SET_ENABLE_CMD_LEN 6
#define TWOSTEP_SET_ENABLE_RESP_LEN 5

#define TWOSTEP_GET_ENABLE 0x17
// stepper_get_enable(uint8_t stepper_num, uint8_t *enable);
#define TWOSTEP_GET_ENABLE_CMD_LEN 5
#define TWOSTEP_GET_ENABLE_RESP_LEN 6

#define TWOSTEP_SET_MICROSTEPS 0x18
//  stepper_set_microsteps(uint8_t stepper_num, uint8_t steppers_microstep_setting stepper_microsteps);
#define TWOSTEP_SET_MICROSTEPS_CMD_LEN 6
#define TWOSTEP_SET_MICROSTEPS_RESP_LEN 5

#define TWOSTEP_GET_MICROSTEPS 0x19
// stepper_get_microsteps(uint8_t stepper_num, enum steppers_microstep_setting *stepper_microsteps);
#define TWOSTEP_GET_MICROSTEPS_CMD_LEN 5
#define TWOSTEP_GET_MICROSTEPS_RESP_LEN 6

#define TWOSTEP_SET_DIR 0x1a
// stepper_set_dir(uint8_t stepper_num, uint8_t dir);
#define TWOSTEP_SET_DIR_CMD_LEN 6
#define TWOSTEP_SET_DIR_RESP_LEN 5

#define TWOSTEP_GET_DIR 0x1b
// stepper_get_dir(uint8_t stepper_num, uint8_t *dir);
#define TWOSTEP_GET_DIR_CMD_LEN 5
#define TWOSTEP_GET_DIR_RESP_LEN 6

#define TWOSTEP_SET_CURRENT 0x1c
// stepper_set_current(uint8_t stepper_num, uint16_t val);
#define TWOSTEP_SET_CURRENT_CMD_LEN 7
#define TWOSTEP_SET_CURRENT_RESP_LEN 5

#define TWOSTEP_GET_CURRENT 0x1d
// stepper_get_current(uint8_t stepper_num, uint16_t *val);
#define TWOSTEP_GET_CURRENT_CMD_LEN 5
#define TWOSTEP_GET_CURRENT_RESP_LEN 7

#define TWOSTEP_SET_100US_DELAY 0x1e
// stepper_set_100uS_delay(uint8_t stepper_num, uint16_t val);
#define TWOSTEP_SET_100US_DELAY_CMD_LEN 7
#define TWOSTEP_SET_100US_DELAY_RESP_LEN 5

#define TWOSTEP_GET_100US_DELAY 0x1f
// stepper_get_100uS_delay(uint8_t stepper_num, uint16_t *val);
#define TWOSTEP_GET_100US_DELAY_CMD_LEN 5
#define TWOSTEP_GET_100US_DELAY_RESP_LEN 7

#define TWOSTEP_GET_RELAY_STATUS 0x30
// uint8_t get_relay_status();
#define TWOSTEP_GET_RELAY_STATUS_CMD_LEN 4
#define TWOSTEP_GET_RELAY_STATUS_RESP_LEN 6

#define TWOSTEP_GET_VERSION 0x40
#define TWOSTEP_GET_VERSION_CMD_LEN 4
#define TWOSTEP_GET_VERSION_RESP_LEN 6

#define TWOSTEP_MIN_CMD_LEN 4
#define TWOSTEP_MIN_RESP_LEN 5
#define TWOSTEP_BAD_CMD_LEN 0
#define TWOSTEP_BAD_RESP_LEN 0


#define TWOSTEP_STEPPER_1 1
#define TWOSTEP_STEPPER_2 2

#define TWOSTEP_STEPPER_BITFIELD_STEPPER_1 1
#define TWOSTEP_STEPPER_BITFIELD_STEPPER_2 2
#define TWOSTEP_STEPPER_BITFIELD_STEPPER_GM 3

#define TWOSTEP_IS_MOVING 0x01
#define TWOSTEP_IS_STOPPED 0x00

#define TWOSTEP_STEPPER_ENABLE 0x01
#define TWOSTEP_STEPPER_DISABLE 0x00

#define TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP 0
#define TWOSTEP_MICROSTEP_BITFIELD_HALF_STEP 1
#define TWOSTEP_MICROSTEP_BITFIELD_QUARTER_STEP 2
#define TWOSTEP_MICROSTEP_BITFIELD_SIXTEENTH_STEP 3

#define TWOSTEP_STEPPER_DIR_HIGH 0x01
#define TWOSTEP_STEPPER_DIR_LOW 0x00

#define TWOSTEP_MAX_CURRENT_VAL 4095
#define TWOSTEP_MIN_CURRENT_VAL 0

#define TWOSTEP_STEP_100US_DELAY_5MS 50
#define TWOSTEP_STEP_100US_DELAY_MIN 1
#define TWOSTEP_STEP_100US_DELAY_MAX (USHRT_MAX)

#define TWOSTEP_RELAYS_R1_A 1
#define TWOSTEP_RELAYS_R1_B 2
#define TWOSTEP_RELAYS_R2_A 4
#define TWOSTEP_RELAYS_R2_B 8
#define TWOSTEP_RELAYS_GC 0xf


uint8_t twostep_cmd_len(uint8_t cmd);
uint8_t twostep_resp_len(uint8_t cmd);

inline void twostep_insert_start_token(uint8_t *buf);
inline bool twostep_verify_start_token(uint8_t *buf);

bool twostep_insert_cmd_end_tokens(uint8_t *ret_buf);
bool twostep_verify_cmd_end_tokens(uint8_t *buf);

bool twostep_insert_resp_end_tokens(uint8_t *ret_buf);
bool twostep_verify_resp_end_tokens(uint8_t *buf);

bool twostep_resp_valid(uint8_t *buf, uint8_t len);

#endif


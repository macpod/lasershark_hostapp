/*
twostep_common_lib.c - Contains constants and functions that
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

#include "twostep_common_lib.h"
#include <string.h>


uint8_t twostep_cmd_len(uint8_t cmd)
{
    uint8_t res = TWOSTEP_BAD_CMD_LEN;

    switch (cmd) {
    case TWOSTEP_SET_STEPS:
        res = TWOSTEP_SET_STEPS_CMD_LEN;
        break;
    case TWOSTEP_SET_SAFE_STEPS:
        res = TWOSTEP_SET_SAFE_STEPS_CMD_LEN;
        break;
    case TWOSTEP_SET_STEP_UNTIL_SWITCH:
        res = TWOSTEP_SET_STEP_UNTIL_SWITCH_CMD_LEN;
        break;
    case TWOSTEP_START:
        res = TWOSTEP_START_CMD_LEN;
        break;
    case TWOSTEP_STOP:
        res = TWOSTEP_STOP_CMD_LEN;
        break;
    case TWOSTEP_GET_IS_MOVING:
        res = TWOSTEP_GET_IS_MOVING_CMD_LEN;
        break;
    case TWOSTEP_SET_ENABLE:
        res = TWOSTEP_SET_ENABLE_CMD_LEN;
        break;
    case TWOSTEP_GET_ENABLE:
        res = TWOSTEP_GET_ENABLE_CMD_LEN;
        break;
    case TWOSTEP_SET_MICROSTEPS:
        res = TWOSTEP_SET_MICROSTEPS_CMD_LEN;
        break;
    case TWOSTEP_GET_MICROSTEPS:
        res = TWOSTEP_GET_MICROSTEPS_CMD_LEN;
        break;
    case TWOSTEP_SET_DIR:
        res = TWOSTEP_SET_DIR_CMD_LEN;
        break;
    case TWOSTEP_GET_DIR:
        res = TWOSTEP_GET_DIR_CMD_LEN;
        break;
    case TWOSTEP_SET_CURRENT:
        res = TWOSTEP_SET_CURRENT_CMD_LEN;
        break;
    case TWOSTEP_GET_CURRENT:
        res = TWOSTEP_GET_CURRENT_CMD_LEN;
        break;
    case TWOSTEP_SET_100US_DELAY:
        res = TWOSTEP_SET_100US_DELAY_CMD_LEN;
        break;
    case TWOSTEP_GET_100US_DELAY:
        res = TWOSTEP_GET_100US_DELAY_CMD_LEN;
        break;
    case TWOSTEP_GET_SWITCH_STATUS:
        res = TWOSTEP_GET_SWITCH_STATUS_CMD_LEN;
        break;
    case TWOSTEP_GET_VERSION:
        res = TWOSTEP_GET_VERSION_CMD_LEN;
        break;
    }

    return res;
}

uint8_t twostep_resp_len(uint8_t cmd)
{
    uint8_t res = TWOSTEP_BAD_RESP_LEN;

    switch (cmd) {
    case TWOSTEP_SET_STEPS:
        res = TWOSTEP_SET_STEPS_RESP_LEN;
        break;
    case TWOSTEP_SET_SAFE_STEPS:
        res = TWOSTEP_SET_SAFE_STEPS_RESP_LEN;
        break;
    case TWOSTEP_SET_STEP_UNTIL_SWITCH:
        res = TWOSTEP_SET_STEP_UNTIL_SWITCH_RESP_LEN;
        break;
    case TWOSTEP_START:
        res = TWOSTEP_START_RESP_LEN;
        break;
    case TWOSTEP_STOP:
        res = TWOSTEP_STOP_RESP_LEN;
        break;
    case TWOSTEP_GET_IS_MOVING:
        res = TWOSTEP_GET_IS_MOVING_RESP_LEN;
        break;
    case TWOSTEP_SET_ENABLE:
        res = TWOSTEP_SET_ENABLE_RESP_LEN;
        break;
    case TWOSTEP_GET_ENABLE:
        res = TWOSTEP_GET_ENABLE_RESP_LEN;
        break;
    case TWOSTEP_SET_MICROSTEPS:
        res = TWOSTEP_SET_MICROSTEPS_RESP_LEN;
        break;
    case TWOSTEP_GET_MICROSTEPS:
        res = TWOSTEP_GET_MICROSTEPS_RESP_LEN;
        break;
    case TWOSTEP_SET_DIR:
        res = TWOSTEP_SET_DIR_RESP_LEN;
        break;
    case TWOSTEP_GET_DIR:
        res = TWOSTEP_GET_DIR_RESP_LEN;
        break;
    case TWOSTEP_SET_CURRENT:
        res = TWOSTEP_SET_CURRENT_RESP_LEN;
        break;
    case TWOSTEP_GET_CURRENT:
        res = TWOSTEP_GET_CURRENT_RESP_LEN;
        break;
    case TWOSTEP_SET_100US_DELAY:
        res = TWOSTEP_SET_100US_DELAY_RESP_LEN;
        break;
    case TWOSTEP_GET_100US_DELAY:
        res = TWOSTEP_GET_100US_DELAY_RESP_LEN;
        break;
    case TWOSTEP_GET_SWITCH_STATUS:
        res = TWOSTEP_GET_SWITCH_STATUS_RESP_LEN;
        break;
    case TWOSTEP_GET_VERSION:
        res = TWOSTEP_GET_VERSION_RESP_LEN;
        break;
    }

    return res;
}


inline void twostep_insert_start_token(uint8_t *buf)
{
    buf[0] = TWOSTEP_START_TOKEN;
}


inline bool twostep_verify_start_token(uint8_t *buf)
{
    return buf[0] == TWOSTEP_START_TOKEN;
}


bool twostep_insert_cmd_end_tokens(uint8_t *cmd_buf)
{
    uint8_t len = twostep_cmd_len(cmd_buf[1]);
    bool res = len > 2;
    if (res) {
        cmd_buf[len-2] = TWOSTEP_END1_TOKEN;
        cmd_buf[len-1] = TWOSTEP_END2_TOKEN;
    }
    return res;
}


bool twostep_verify_cmd_end_tokens(uint8_t *cmd_buf)
{
    uint8_t len = twostep_cmd_len(cmd_buf[1]);
    bool res = len > 2;
    if (res) {
        res = cmd_buf[len-2] == TWOSTEP_END1_TOKEN && cmd_buf[len-1] == TWOSTEP_END2_TOKEN;
    }
    return res;
}


bool twostep_insert_resp_end_tokens(uint8_t *resp_buf)
{
    uint8_t len = twostep_resp_len(resp_buf[1]);
    bool res = len > 2;
    if (res) {
        resp_buf[len-2] = TWOSTEP_END1_TOKEN;
        resp_buf[len-1] = TWOSTEP_END2_TOKEN;
    }
    return res;
}


bool twostep_verify_resp_end_tokens(uint8_t *resp_buf)
{
    uint8_t len = twostep_resp_len(resp_buf[1]);
    bool res = len > 2;
    if (res) {
        res = resp_buf[len-2] == TWOSTEP_END1_TOKEN && resp_buf[len-1] == TWOSTEP_END2_TOKEN;
    }
    return res;
}


bool twostep_resp_valid(uint8_t *buf, uint8_t len)
{
    bool res = len >= TWOSTEP_MIN_RESP_LEN;
    if (res) {
        res = twostep_verify_start_token(buf);
    }
    if (res) {
        res = TWOSTEP_BAD_RESP_LEN != twostep_resp_len(buf[1]);
    }
    if (res) {
        res = twostep_verify_resp_end_tokens(buf);
    }
    if (res) {
        res = len == twostep_resp_len(buf[1]);
    }

    return res;
}



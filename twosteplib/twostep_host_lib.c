/*
twostep_host_lib.c - Contains functions that assist with 
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

#include "twostep_host_lib.h"
#include <string.h>


bool twostep_resp_indicates_passed(uint8_t *resp_buf, uint8_t len)
{
    bool res = true;

    if (resp_buf[2] != TWOSTEP_CMD_SUCCESS) {
        res = false;
    }

    return res;
}


uint8_t twostep_cmd_set_steps(uint8_t *buf, uint8_t stepper_num, uint32_t steps)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_STEPS;
    buf[i++] = stepper_num;
    memcpy(buf+i, &steps, sizeof(uint32_t));
    i+=sizeof(uint32_t);
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_safe_steps(uint8_t *buf, uint8_t stepper_num, uint32_t steps)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_SAFE_STEPS;
    buf[i++] = stepper_num;
    memcpy(buf+i, &steps, sizeof(uint32_t));
    i+=sizeof(uint32_t);
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_step_until_switch(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_STEP_UNTIL_SWITCH;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_start(uint8_t *buf, uint8_t stepper_bitfield)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_START;
    buf[i++] = stepper_bitfield;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_stop(uint8_t *buf, uint8_t stepper_bitfield)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_STOP;
    buf[i++] = stepper_bitfield;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_is_moving(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_IS_MOVING;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


bool twostep_resp_get_is_moving(uint8_t *buf, uint8_t *is_moving)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_IS_MOVING) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        if (u8_temp == TWOSTEP_IS_MOVING || u8_temp == TWOSTEP_IS_STOPPED) {
            *is_moving = u8_temp;
        } else {
            res = false;
        }
    }
    return res;
}


uint8_t twostep_cmd_set_enable(uint8_t *buf, uint8_t stepper_num, bool enable)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_ENABLE;
    buf[i++] = stepper_num;
    buf[i++] = enable ? TWOSTEP_STEPPER_ENABLE : TWOSTEP_STEPPER_DISABLE;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_microsteps(uint8_t *buf, uint8_t stepper_num, uint8_t microstep_bitfield)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_MICROSTEPS;
    buf[i++] = stepper_num;
    buf[i++] = microstep_bitfield;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_microsteps(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_MICROSTEPS;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_enable(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_ENABLE;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_dir(uint8_t *buf, uint8_t stepper_num, bool high)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_DIR;
    buf[i++] = stepper_num;
    buf[i++] = high ? TWOSTEP_STEPPER_DIR_HIGH : TWOSTEP_STEPPER_DIR_LOW;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_dir(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_DIR;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_current(uint8_t *buf, uint8_t stepper_num, uint16_t current)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_CURRENT;
    buf[i++] = stepper_num;
    memcpy(buf+i, &current, sizeof(uint16_t));
    i+=sizeof(uint16_t);
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_current(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_CURRENT;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_set_100uS_delay(uint8_t *buf, uint8_t stepper_num, uint16_t delay)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_SET_100US_DELAY;
    buf[i++] = stepper_num;
    memcpy(buf+i, &delay, sizeof(uint16_t));
    i+=sizeof(uint16_t);
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_100uS_delay(uint8_t *buf, uint8_t stepper_num)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_100US_DELAY;
    buf[i++] = stepper_num;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_switch_status(uint8_t *buf)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_SWITCH_STATUS;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


uint8_t twostep_cmd_get_version (uint8_t *buf)
{
    int i = 1;
    twostep_insert_start_token(buf);
    buf[i++] = TWOSTEP_GET_VERSION;
    twostep_insert_cmd_end_tokens(buf);
    i+=2;
    return i;
}


bool twostep_resp_get_enable(uint8_t *buf,uint8_t *enable)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_ENABLE) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        if (u8_temp == TWOSTEP_STEPPER_ENABLE || u8_temp == TWOSTEP_STEPPER_DISABLE) {
            *enable = u8_temp;
        } else {
            res = false;
        }
    }
    return res;
}


bool twostep_resp_get_microsteps(uint8_t *buf,uint8_t *microstep_bitfield)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_MICROSTEPS) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        if (u8_temp == TWOSTEP_MICROSTEP_BITFIELD_FULL_STEP ||
                u8_temp == TWOSTEP_MICROSTEP_BITFIELD_HALF_STEP ||
                u8_temp == TWOSTEP_MICROSTEP_BITFIELD_QUARTER_STEP ||
                u8_temp == TWOSTEP_MICROSTEP_BITFIELD_SIXTEENTH_STEP) {
            *microstep_bitfield = u8_temp;
        } else {
            res = false;
        }
    }

    return res;
}


bool twostep_resp_get_dir(uint8_t *buf,uint8_t *high)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_DIR) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        if (u8_temp == TWOSTEP_STEPPER_DIR_HIGH || u8_temp == TWOSTEP_STEPPER_DIR_LOW) {
            *high = u8_temp;
        } else {
            res = false;
        }
    }
    return res;
}


bool twostep_resp_get_current(uint8_t *buf,uint16_t *current)
{
    int i = 3;
    bool res = true;
    uint16_t u16_temp;
    if (buf[1] != TWOSTEP_GET_CURRENT) {
        res = false;
    }

    if (res) {
        memcpy(&u16_temp, buf+i, sizeof(uint16_t));
        i+=sizeof(uint16_t);
        if (u16_temp >= TWOSTEP_MIN_CURRENT_VAL && u16_temp <= TWOSTEP_MAX_CURRENT_VAL) {
            *current = u16_temp;
        } else {
            res = false;
        }
    }
    return res;
}


bool twostep_resp_get_100uS_delay(uint8_t *buf,uint16_t *delay)
{
    int i = 3;
    bool res = true;
    uint16_t u16_temp;
    if (buf[1] != TWOSTEP_GET_100US_DELAY) {
        res = false;
    }

    if (res) {
        memcpy(&u16_temp, buf+i, sizeof(uint16_t));
        if (u16_temp >= TWOSTEP_STEP_100US_DELAY_MIN && u16_temp <= TWOSTEP_STEP_100US_DELAY_MAX) {
            *delay = u16_temp;
        } else {
            res = false;
        }
    }

    return res;
}


bool twostep_resp_get_switch_status(uint8_t *buf, uint8_t *switch_bitfield)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_SWITCH_STATUS) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        if ((u8_temp & TWOSTEP_SWITCHES_GC) == u8_temp) {
            *switch_bitfield = u8_temp;
        } else {
            res = false;
        }
    }

    return res;
}


bool twostep_resp_get_version(uint8_t *buf, uint8_t *version)
{
    int i = 3;
    bool res = true;
    uint8_t u8_temp;
    if (buf[1] != TWOSTEP_GET_VERSION) {
        res = false;
    }

    if (res) {
        u8_temp = buf[i++];
        *version = u8_temp;
    }
    return res;
}


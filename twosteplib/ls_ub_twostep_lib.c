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

#include "ls_ub_twostep_lib.h"
#include "../lasersharklib/lasershark_uart_bridge_lib.h"
#include "twostep_host_lib.h"
#include <stdio.h>
#include <unistd.h>


static void __nsleep(const struct timespec *req, struct timespec *rem)
{
    struct timespec temp_rem;
    if(nanosleep(req,rem)==-1)
        __nsleep(rem,&temp_rem);
}


static void msleep(unsigned long milisec)
{
    struct timespec req= {0},rem= {0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    __nsleep(&req,&rem);
}


static int ls_ub_twostep_tx_helper(libusb_device_handle *devh_ub, uint8_t *buf, uint8_t tx_len)
{
    int res = LS_UB_TWOSTEP_SUCCESS;
    uint8_t temp_len;
    if (LASERSHARK_UB_CMD_SUCCESS != lasershark_ub_tx(devh_ub, tx_len, buf, &temp_len)) {
        res = LS_UB_TWOSTEP_UB_FAIL;
    }

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (temp_len != tx_len) {
            res = LS_UB_TWOSTEP_TX_FAIL;
        }
    }

    return res;
}


static int ls_ub_twostep_rx_helper(libusb_device_handle *devh_ub, uint8_t *buf, uint8_t rx_expected_len)
{
    int res = LS_UB_TWOSTEP_SUCCESS;
    uint8_t temp_len;

    msleep(10); // This is ugly and should be fixed with later versions. A polling option would be good.

    if (LASERSHARK_UB_CMD_SUCCESS != lasershark_ub_rx(devh_ub, rx_expected_len, buf, &temp_len)) {
        res = LS_UB_TWOSTEP_UB_FAIL;
    }

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (temp_len != rx_expected_len) {
            res = LS_UB_TWOSTEP_RX_FAIL;
        }
    }

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (!twostep_resp_valid(buf, rx_expected_len)) {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }

    return res;
}


static int ls_ub_twostep_cmd_send_helper(libusb_device_handle *devh_ub, uint8_t *buf, uint8_t len)
{
    bool res;
    lasershark_ub_clear_rx_fifo(devh_ub);

    res = ls_ub_twostep_tx_helper(devh_ub, buf, len);
    if (res == LS_UB_TWOSTEP_SUCCESS) {
        len = twostep_resp_len(buf[1]);
        res = ls_ub_twostep_rx_helper(devh_ub, buf, len);
    }

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (!twostep_resp_indicates_passed(buf, len)) {
            res = LS_UB_TWOSTEP_TS_CMD_FAIL;
        }
    }
    return res;
}


int ls_ub_twostep_set_steps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint32_t steps)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_steps(buf, stepper_num, steps);
    lasershark_ub_clear_rx_fifo(devh_ub);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_set_safe_steps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint32_t steps)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_safe_steps(buf, stepper_num, steps);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_set_step_until_switch(libusb_device_handle *devh_ub, uint8_t stepper_num)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_step_until_switch(buf, stepper_num);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}



int ls_ub_twostep_start(libusb_device_handle *devh_ub, uint8_t stepper_bitfield)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_start(buf, stepper_bitfield);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_stop(libusb_device_handle *devh_ub, uint8_t stepper_bitfield)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_stop(buf, stepper_bitfield);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_get_is_moving(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *is_moving)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_is_moving(buf, stepper_num);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_is_moving(buf, &u8_temp)) {
            *is_moving = u8_temp == TWOSTEP_IS_MOVING ? true : false;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


int ls_ub_twostep_set_enable(libusb_device_handle *devh_ub, uint8_t stepper_num, bool enable)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_enable(buf, stepper_num, enable);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_get_enable(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *enable)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_enable(buf, stepper_num);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_enable(buf, &u8_temp)) {
            *enable = u8_temp == TWOSTEP_STEPPER_ENABLE ? true : false;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


int ls_ub_twostep_set_microsteps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint8_t microstep_bitfield)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_microsteps(buf, stepper_num, microstep_bitfield);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_get_microsteps(libusb_device_handle *devh_ub, uint8_t stepper_num, uint8_t *microstep_bitfield)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_microsteps(buf, stepper_num);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_microsteps(buf, &u8_temp)) {
            *microstep_bitfield = u8_temp;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


int ls_ub_twostep_set_dir(libusb_device_handle *devh_ub, uint8_t stepper_num, bool high)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_dir(buf, stepper_num, high);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


int ls_ub_twostep_get_dir(libusb_device_handle *devh_ub, uint8_t stepper_num, bool *high)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_dir(buf, stepper_num);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_dir(buf, &u8_temp)) {
            *high = u8_temp == TWOSTEP_STEPPER_DIR_HIGH ? true : false;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


uint8_t ls_ub_twostep_set_current(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t current)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_current(buf, stepper_num, current);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


uint8_t ls_ub_twostep_get_current(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t *current)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_current(buf, stepper_num);
    uint16_t u16_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_current(buf, &u16_temp)) {
            *current = u16_temp;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


uint8_t ls_ub_twostep_set_100uS_delay(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t delay)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_set_100uS_delay(buf, stepper_num, delay);
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    return res;
}


uint8_t ls_ub_twostep_get_100uS_delay(libusb_device_handle *devh_ub, uint8_t stepper_num, uint16_t *delay)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_100uS_delay(buf, stepper_num);
    uint16_t u16_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_100uS_delay(buf, &u16_temp)) {
            *delay = u16_temp;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


uint8_t ls_ub_twostep_get_switch_status(libusb_device_handle *devh_ub, uint8_t *switch_bitfield)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_switch_status(buf);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);
    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_switch_status(buf, &u8_temp)) {
            *switch_bitfield = u8_temp;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}


uint8_t ls_ub_twostep_get_version(libusb_device_handle *devh_ub, uint8_t *version)
{
    int res;
    uint8_t buf[TWOSTEP_BUF_SIZE];
    uint8_t len = twostep_cmd_get_version(buf);
    uint8_t u8_temp = 0;
    res = ls_ub_twostep_cmd_send_helper(devh_ub, buf, len);

    if (res == LS_UB_TWOSTEP_SUCCESS) {
        if (twostep_resp_get_version(buf, &u8_temp)) {
            *version = u8_temp;
        } else {
            res = LS_UB_TWOSTEP_TS_PROTO_FAIL;
        }
    }
    return res;
}



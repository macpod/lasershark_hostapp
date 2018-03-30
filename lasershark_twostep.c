/*
lasershark_twostep.c - Allows communications to Twostep board via
the Lasershark board's UART.
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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <libusb.h>
#include <signal.h>
#include <time.h>
#include "twosteplib/ls_ub_twostep_lib.h"
#include "lasersharklib/lasershark_uart_bridge_lib.h"

#define LASERSHARK_VIN 0x1fc9
#define LASERSHARK_PID 0x04d8

int do_exit = 0;
pid_t pid;


int lasershark_serialnum_len = 64;
unsigned char lasershark_serialnum[64];
uint32_t lasershark_ub_version = 0;
uint8_t lasershark_ub_max_rx = 0;
uint8_t lasershark_ub_max_tx = 0;


struct libusb_device_handle *devh_ub = NULL;

sigset_t mask, oldmask;


static void sig_hdlr(int signum)
{
    switch (signum) {
    case SIGINT:
        printf("\nGot request to quit\n");
        do_exit = 1;
        break;
    case SIGUSR1:
        printf("sigusr1 caught\n");
        do_exit = 1;
        break;
    default:
        printf("what\n");
    }
}


void quit_program()
{
    kill(pid, SIGUSR1);
}


void tests()
{

    bool enable;
    uint8_t microsteps;
    bool moving;
    bool high;
    uint16_t current;
    uint16_t delay;
    uint8_t switches;
    uint8_t version;


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_enable(devh_ub, TWOSTEP_STEPPER_1, true)) {
        printf("set enable cmd failed\n");
    } else {
        printf("set enable cmd passed\n");
    }
    printf("\n");

    sleep(1);

    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_enable(devh_ub, TWOSTEP_STEPPER_1, &enable)) {
        printf("get enable cmd failed\n");
    } else {
        printf("get enable cmd passed enable state is: %d\n", enable);
    }
    printf("\n");
    sleep(1);

    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_safe_steps(devh_ub, TWOSTEP_STEPPER_1, 2000)) {
        printf("set steps cmd failed\n");
    } else {
        printf("set steps cmd passed\n");
    }
    printf("\n");
    sleep(1);



    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_safe_steps(devh_ub, TWOSTEP_STEPPER_1, 2000)) {
        printf("set safe steps cmd failed\n");
    } else {
        printf("set safe steps cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_step_until_switch(devh_ub, TWOSTEP_STEPPER_1)) {
        printf("set step until switch cmd failed\n");
    } else {
        printf("set step until switch cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_microsteps(devh_ub, TWOSTEP_STEPPER_1, TWOSTEP_MICROSTEP_BITFIELD_SIXTEENTH_STEP)) {
        printf("set microsteps cmd failed\n");
    } else {
        printf("set microsteps cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_microsteps(devh_ub, TWOSTEP_STEPPER_1, &microsteps)) {
        printf("get microsteps cmd failed\n");
    } else {
        printf("get microsteps cmd passed. microsteps val is: %d\n", microsteps);
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_dir(devh_ub, TWOSTEP_STEPPER_1, false)) {
        printf("set dir cmd failed\n");
    } else {
        printf("set dir cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_dir(devh_ub, TWOSTEP_STEPPER_1, &high)) {
        printf("get stepper dir cmd failed\n");
    } else {
        printf("get stepper dir cmd passed. high state is: %d\n", high);
    }
    printf("\n");
    sleep(1);



    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_current(devh_ub, TWOSTEP_STEPPER_1, 200)) {
        printf("set current cmd failed\n");
    } else {
        printf("set current cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_current(devh_ub, TWOSTEP_STEPPER_1, &current)) {
        printf("get current cmd failed\n");
    } else {
        printf("get current cmd passed. current val is: %d\n", current);
    }
    printf("\n");
    sleep(1);



    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_set_100uS_delay(devh_ub, TWOSTEP_STEPPER_1, 1000)) {
        printf("set 100uS delay cmd failed\n");
    } else {
        printf("set 100uS delay cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_100uS_delay(devh_ub, TWOSTEP_STEPPER_1, &delay)) {
        printf("get 100uS delay cmd failed\n");
    } else {
        printf("get 100uS delay cmd passed. delay val is: %d\n", delay);
    }
    printf("\n");
    sleep(1);



    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_switch_status(devh_ub, &switches)) {
        printf("get switch status cmd failed\n");
    } else {
        printf("get switch status cmd passed. switch val is: %02x\n", switches);
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_version(devh_ub, &version)) {
        printf("get version cmd failed\n");
    } else {
        printf("get version cmd passed. version val is: %d\n", version);
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_start(devh_ub, TWOSTEP_STEPPER_BITFIELD_STEPPER_1)) {
        printf("start cmd failed\n");
    } else {
        printf("start cmd passed\n");
    }
    printf("\n");
    sleep(1);


    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_get_is_moving(devh_ub, TWOSTEP_STEPPER_1, &moving)) {
        printf("get is moving cmd failed\n");
    } else {
        printf("get is moving cmd passed. Moving is: %d\n", moving);
    }
    printf("\n");
    sleep(1);



    if (LS_UB_TWOSTEP_SUCCESS != ls_ub_twostep_stop(devh_ub, TWOSTEP_STEPPER_BITFIELD_STEPPER_1)) {
        printf("stop cmd failed\n");
    } else {
        printf("stop cmd passed\n");
    }
    printf("\n");
    sleep(1);
}


int main(int argc, char *argv[])
{
    int rc;
    struct sigaction sigact;
    bool high[2];
    bool moving;
    int i;

    pid = getpid();
    sigact.sa_handler = sig_hdlr;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);


    rc = libusb_init(NULL);
    if (rc < 0) {
        fprintf(stderr, "Error initializing libusb: %d\n", /*libusb_error_name(rc)*/rc);
        exit(1);
    }

    devh_ub = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    if (!devh_ub) {
        fprintf(stderr, "Error finding USB device\n");
        goto out_post_release;
    }


    libusb_set_debug(NULL, 3);

    rc = libusb_claim_interface(devh_ub, 2);
    if (rc < 0) {
        fprintf(stderr, "Error claiming uart bridge interface: %d\n", /*libusb_error_name(rc)*/rc);
        goto out_post_release;
    }


    struct libusb_device_descriptor desc;

    rc = libusb_get_device_descriptor(libusb_get_device(devh_ub), &desc);
    if (rc < 0) {
        fprintf(stderr, "Error obtaining device descriptor: %d\n", /*libusb_error_name(rc)*/rc);
    }

    memset(lasershark_serialnum, 0, lasershark_serialnum_len);
    rc = libusb_get_string_descriptor_ascii(devh_ub, desc.iSerialNumber, lasershark_serialnum, lasershark_serialnum_len);
    if (rc < 0) {
        fprintf(stderr, "Error obtaining iSerialNumber: %d\n", /*libusb_error_name(rc)*/rc);
    }
    printf("iSerialNumber: %s\n", lasershark_serialnum);


    rc = lasershark_ub_get_version(devh_ub, &lasershark_ub_version);
    if (rc != LASERSHARK_UB_CMD_SUCCESS) {
        printf("Getting UB version failed.\n");
        goto out;
    }
    printf("Getting UB version: %d\n", lasershark_ub_version);


    rc = lasershark_ub_get_max_rx(devh_ub, &lasershark_ub_max_rx);
    if (rc != LASERSHARK_UB_CMD_SUCCESS) {
        printf("Getting UB max RX failed.\n");
        goto out;
    }
    printf("Getting UB max RX: %d\n", lasershark_ub_max_rx);

    rc = lasershark_ub_get_max_tx(devh_ub, &lasershark_ub_max_rx);
    if (rc != LASERSHARK_UB_CMD_SUCCESS) {
        printf("Getting UB max TX failed.\n");
        goto out;
    }
    printf("Getting UB max TX: %d\n", lasershark_ub_max_rx);


    rc = lasershark_ub_get_max_tx(devh_ub, &lasershark_ub_max_rx);
    if (rc != LASERSHARK_UB_CMD_SUCCESS) {
        printf("Clearing UB RX fifo failed.\n");
        goto out;
    }
    printf("Clearing UB RX FIFO worked\n");


    sigemptyset (&mask);
    sigaddset (&mask, SIGUSR1);

    sigprocmask (SIG_BLOCK, &mask, &oldmask);


    printf("Running\n");


    // Just example code. Not going to bother verifying if these worked or not.
    do_exit = ls_ub_twostep_set_enable(devh_ub, TWOSTEP_STEPPER_1, true);
    printf("Set enable 1: %d\n", do_exit);
    do_exit = ls_ub_twostep_set_enable(devh_ub, TWOSTEP_STEPPER_2, true);
    printf("Set enable 2: %d\n", do_exit);
    do_exit = ls_ub_twostep_set_100uS_delay(devh_ub, TWOSTEP_STEPPER_1, 100);
    printf("Set delay 1: %d\n", do_exit);
    do_exit = ls_ub_twostep_set_100uS_delay(devh_ub, TWOSTEP_STEPPER_2, 500);
    printf("Set delay 2: %d\n", do_exit);


    high[1] = false;
    high[2] = true;

    do {
        for (i = 0; i < 2 && !do_exit; i++) {
            do_exit = ls_ub_twostep_get_is_moving(devh_ub, i == 1 ? TWOSTEP_STEPPER_1 : TWOSTEP_STEPPER_2, &moving);
            printf("get is moving %d %d: %d\n", i, moving, do_exit);

            if (!do_exit && !moving) {
                high[i] = !high[i];
                do_exit = ls_ub_twostep_set_dir(devh_ub, i == 1 ? TWOSTEP_STEPPER_1 : TWOSTEP_STEPPER_2, high[i]);
                printf("set dir %d: %d\n", i, do_exit);

                if (!do_exit) {
                    do_exit = ls_ub_twostep_set_safe_steps(devh_ub, i == 1 ? TWOSTEP_STEPPER_1 : TWOSTEP_STEPPER_2, 200);
                    printf("set safe steps %d: %d\n", i, do_exit);
                }

                if (!do_exit) {
                    do_exit = ls_ub_twostep_start(devh_ub, i == 1 ? TWOSTEP_STEPPER_1 : TWOSTEP_STEPPER_2);
                    printf("start %d: %d\n", i, do_exit);
                }
            }
        }
        printf("loop\n");
        sleep(1);
    } while (!do_exit);

    printf("Quitting gracefully\n");
    do_exit = ls_ub_twostep_stop(devh_ub, TWOSTEP_STEPPER_1);
    printf("start 1: %d\n", do_exit);
    do_exit = ls_ub_twostep_stop(devh_ub, TWOSTEP_STEPPER_2);
    printf("start 2: %d\n", do_exit);
    do_exit = ls_ub_twostep_set_enable(devh_ub, TWOSTEP_STEPPER_1, false);
    printf("Set disable 1: %d\n", do_exit);
    do_exit = ls_ub_twostep_set_enable(devh_ub, TWOSTEP_STEPPER_2, false);
    printf("Set disable 2: %d\n", do_exit);

// THINGS COME HERE TO DIE!!!!!!!!!!!!!!!!!!!
out:
    libusb_release_interface(devh_ub, 0);

out_post_release:
    if (devh_ub) {
        libusb_close(devh_ub);
    }
    libusb_exit(NULL);


    return rc;
}


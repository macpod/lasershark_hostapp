/*
lasershark_3d_printer_lib.h - Library to help 3d host apps talk with Lasershark devices.
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

#ifndef LASERSHARK_3D_PRINTER_LIB_H_
#define LASERSHARK_3D_PRINTER_LIB_H_

#include "lasershark_lib.h"

// Steps stepper in specified direction for supplied number of steps.
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_1_STEP_TOWARDS_HOME 0XA0
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_1_STEP_AWAY_FROM_HOME 0XA1
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_2_STEP_TOWARDS_HOME 0XA2
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_2_STEP_AWAY_FROM_HOME 0XA3

// Steps stepper(s) to home position based on given param.
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_HOME 0XA4
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_HOME_STEPPER_1 0x01
#define LASERSHARK_3D_PRINTER_CMD_STEPPER_HOME_STEPPER_2 0x02

// Gets value of relays
#define LASERSHARK_3D_PRINTER_CMD_GET_R1 0xA5
#define LASERSHARK_3D_PRINTER_CMD_GET_R2 0xA6
#define LASERSHARK_3D_PRINTER_CMD_GET_R_STATE_LOW 0x00
#define LASERSHARK_3D_PRINTER_CMD_GET_R_STATE_HIGH 0x01


// Sets stepper home direction dir pin val.
#define LASERSHARK_3D_PRINTER_CMD_SET_STEPPER_1_HOME_DIR 0XA7
#define LASERSHARK_3D_PRINTER_CMD_SET_STEPPER_2_HOME_DIR 0XA8
#define LASERSHARK_3D_PRINTER_CMD_DIR_STATE_LOW 0x00
#define LASERSHARK_3D_PRINTER_CMD_DIR_STATE_HIGH 0x01

// Sets stepper motor step delay (in ms).
#define LASERSHARK_3D_PRINTER_STEPPER_SET_SET_DELAY_MS 0XA9

// Move steppers by X steps.
int stepper_1_step_towards_home(libusb_device_handle *devh_ctl, uint32_t steps);
int stepper_1_step_away_from_home(libusb_device_handle *devh_ctl, uint32_t steps);
int stepper_2_step_towards_home(libusb_device_handle *devh_ctl, uint32_t steps);
int stepper_2_step_away_from_home(libusb_device_handle *devh_ctl, uint32_t steps);

// Move specified steppers home.
int stepper_home(libusb_device_handle *devh_ctl, uint32_t steppers);

// Get state of relays
int get_r1(libusb_device_handle *devh_ctl, uint8_t *state);
int get_r2(libusb_device_handle *devh_ctl, uint8_t *state);

// Set stepper motor home direction.
int set_stepper_1_home_dir(libusb_device_handle *devh_ctl, uint8_t dir);
int set_stepper_2_home_dir(libusb_device_handle *devh_ctl, uint8_t dir);

// Set delay in MS between motor steps and dir changes.
int set_stepper_step_delay_ms(libusb_device_handle *devh_ctl, uint32_t delay);



#endif /* LASERSHARK_3D_PRINTER_LIB_H_ */

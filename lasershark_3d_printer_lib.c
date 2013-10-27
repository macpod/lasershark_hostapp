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

#include "lasershark_3d_printer_lib.h"

int stepper_1_step_towards_home(libusb_device_handle *devh_ctl, uint32_t steps)
{
    return lasershark_set_uint32(devh_ctl, LASERSHARK_3D_PRINTER_CMD_STEPPER_1_STEP_TOWARDS_HOME, steps);
}

int stepper_1_step_away_from_home(libusb_device_handle *devh_ctl, uint32_t steps)
{
    return lasershark_set_uint32(devh_ctl, LASERSHARK_3D_PRINTER_CMD_STEPPER_1_STEP_AWAY_FROM_HOME, steps);
}

int stepper_2_step_towards_home(libusb_device_handle *devh_ctl, uint32_t steps)
{
    return lasershark_set_uint32(devh_ctl, LASERSHARK_3D_PRINTER_CMD_STEPPER_2_STEP_TOWARDS_HOME, steps);
}

int stepper_2_step_away_from_home(libusb_device_handle *devh_ctl, uint32_t steps)
{
    return lasershark_set_uint32(devh_ctl, LASERSHARK_3D_PRINTER_CMD_STEPPER_2_STEP_AWAY_FROM_HOME, steps);
}

int stepper_home(libusb_device_handle *devh_ctl, uint8_t steppers)
{
    return lasershark_set_uint8(devh_ctl, LASERSHARK_3D_PRINTER_CMD_STEPPER_HOME, steppers);
}

int get_r1(libusb_device_handle *devh_ctl, uint8_t *state)
{
    return lasershark_get_uint8(devh_ctl, LASERSHARK_3D_PRINTER_CMD_GET_R1, state);
}

int get_r2(libusb_device_handle *devh_ctl, uint8_t *state)
{
    return lasershark_get_uint8(devh_ctl, LASERSHARK_3D_PRINTER_CMD_GET_R2, state);
}

int set_stepper_1_home_dir(libusb_device_handle *devh_ctl, uint8_t dir)
{
    return lasershark_set_uint8(devh_ctl, LASERSHARK_3D_PRINTER_CMD_SET_STEPPER_1_HOME_DIR, dir);
}

int set_stepper_2_home_dir(libusb_device_handle *devh_ctl, uint8_t dir)
{
    return lasershark_set_uint8(devh_ctl, LASERSHARK_3D_PRINTER_CMD_SET_STEPPER_2_HOME_DIR, dir);
}

int set_stepper_step_delay_ms(libusb_device_handle *devh_ctl, uint32_t delay)
{
    return lasershark_set_uint32(devh_ctl, LASERSHARK_3D_PRINTER_STEPPER_SET_SET_DELAY_MS, delay);
}


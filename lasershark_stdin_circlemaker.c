/*
lasershark_stdin_circlemaker.c - Application that draws a circle intended
for consumption by lasershark_stdin.
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
#include <stdio.h>
#include <stdint.h>
#include <math.h>

static uint16_t float_to_lasershark_xy(float var)
{
    uint16_t val = (4095 * (var + 1.0)/2.0);
    return val;
}


int main (int argc, char *argv[])
{
    double x_f, y_f;
    uint32_t index = 0;
    float step = 2*M_PI/1000;
    printf("r=20000\n");
    printf("e=1\n");

    x_f = 0;
    y_f = 0;

    while(1) {
        x_f = sinf(index*step);
        y_f = cosf(index*step);
        printf("s=%u,%u,%u,%u,%u,%u\n",
               float_to_lasershark_xy(x_f),  float_to_lasershark_xy(y_f), 4095,4095,1,1); // x, y, a, b, c, intl_a
        index++;
    }

    // These should really be stuck at the end of the output.. but this is just a demo.
    //printf("f=1\n");
    //printf("e=0\n");
    return 0;
}


/*
lasershark_stdin_displayimage.c - Application that draws a given png
for consumption by lasershark_stdin.
Copyright (C) 2015 Jeffrey Nelson <nelsonjm@macpod.net>

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
#include <stdlib.h>
#include "getopt_portable.h"
#include "lodepng/lodepng.h"

#define MIN_VAL 0
#define MID_VAL 2048
#define MAX_VAL 4095

#define MAX_WIDTH 4096
#define MAX_HEIGHT 4096

void print_help(const char* prog_name, FILE* stream)
{
    fprintf(stream, "%s [OPTIONS] - Displays an image via LaserShark\n", prog_name);
    fprintf(stream, "\t-h");
    fprintf(stream, "\tPrint this help text\n");
    fprintf(stream, "\t-m");
    fprintf(stream, "\tPrint image in monochrome\n");
    fprintf(stream, "\t-g");
    fprintf(stream, "\tPrint image in greyscale\n");
    fprintf(stream, "\t-x");
    fprintf(stream, "\tPrint image in rgb\n");
    fprintf(stream, "\t-a");
    fprintf(stream, "\tA-channel minimum value\n");
    fprintf(stream, "\t-A");
    fprintf(stream, "\tA-channel maximum value\n");
    fprintf(stream, "\t-b");
    fprintf(stream, "\tB-channel minimum value\n");
    fprintf(stream, "\t-B");
    fprintf(stream, "\tB-channel maximum value\n");
    fprintf(stream, "\t-p\n");
    fprintf(stream, "\tPNG to print. Must be less than or equal to 4096x4096 in size\n");
    fprintf(stream, "\t-r\n");
    fprintf(stream, "\tRate to display samples at. Must be between 1 and 30,000\n");
}


int main (int argc, char *argv[])
{
    int rc;
    int ret = 1;
    int c;
    unsigned int w, h;
    unsigned int w_off, h_off;
    uint16_t *image;

    int sample_count;
    int count = 0;
    int tmp_pos;
    unsigned int curr_x_pos = 0, curr_y_pos = 0;
    unsigned int a_min = MIN_VAL;
    unsigned int a_max = MAX_VAL;
    unsigned int b_min = MIN_VAL;
    unsigned int b_max = MAX_VAL;
    unsigned int a_val;
    unsigned int b_val;
    unsigned int c_val;

    int aflag = 0;
    int Aflag = 0;
    int bflag = 0;
    int Bflag = 0;
    int hflag = 0;
    int mflag = 0;
    int gflag = 0;
    int xflag = 0;
    int pflag = 0;
    char* path = NULL;
    int rflag = 0;
    int rate = 20000;


    opterr_portable = 1;
    while (-1 != (c =getopt_portable(argc, argv, "a:A:b:B:hmgxp:r:"))) {
        switch(c) {
        case 'a':
            aflag++;
            a_min = atoi(optarg_portable);
            break;
        case 'A':
            Aflag++;
            a_max = atoi(optarg_portable);
            break;
        case 'b':
            bflag++;
            b_min = atoi(optarg_portable);
            break;
        case 'B':
            Bflag++;
            b_max = atoi(optarg_portable);
            break;
        case 'h':
            hflag++;
            break;
        case 'm':
            mflag++;
            break;
        case 'g':
            gflag++;
            break;
        case 'x':
            xflag++;
            break;
        case 'p':
            pflag++;
            path = optarg_portable;
            break;
        case 'r':
            rflag++;
            rate = atoi(optarg_portable);
            break;
        default:
            print_help(argv[0], stderr);
            exit(1);
        }
    }


    if (aflag > 1 || Aflag > 1 || bflag > 1 || Bflag > 1 ||
            hflag > 1 ||
            mflag > 1 || gflag > 1 || xflag > 1 || xflag > 1 || rflag > 1 || pflag > 1) {
        fprintf(stderr, "Cannot specify flags more than once.\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (mflag && gflag && xflag) {
        fprintf(stderr, "Only -m, -g, -r -x flag may be specified.\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (!mflag && !gflag && !xflag) {
        fprintf(stderr, "Must specify -m, -g, -r -c flag.\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (!pflag) {
        fprintf(stderr, "Must specify image to print\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (a_min < MIN_VAL || a_min > MAX_VAL || b_min < MIN_VAL || a_max > MAX_VAL) {
        fprintf(stderr, "A-channel min and max must be between %d and %d\n", MIN_VAL, MAX_VAL);
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (a_min > a_max) {
        fprintf(stderr, "A-channel min cannot be greater than the A-channel max\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (b_min < MIN_VAL || b_min > MAX_VAL || b_max < MIN_VAL || b_max > MAX_VAL) {
        fprintf(stderr, "B-channel min and max must be between %d and %d\n", MIN_VAL, MAX_VAL);
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (b_min > b_max) {
        fprintf(stderr, "B-channel min cannot be greater than the B-channel max\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (rate < 0 || rate > 30000) {
        fprintf(stderr, "Rate must be between 1 and 30,000\n");
        print_help(argv[0], stderr);
        goto out_post;
    }

    if (hflag) {
        print_help(argv[0], stdout);
        goto out_post;
    }


    rc = lodepng_decode_file((unsigned char**)&image, &w, &h, path, LCT_RGB, 16);
    if (rc) {
        fprintf(stderr, "Error opening image: %s\n", lodepng_error_text(rc));
        goto out_post;
    }

    if (w > MAX_WIDTH || h > MAX_HEIGHT) {
        fprintf(stderr, "Image cannot be larger than 4096 pixels in width or height\n");
        goto out_post;
    }

    w_off = (MAX_WIDTH - w) / 2;
    h_off = (MAX_HEIGHT - h) / 2;
    sample_count = w * h;

    printf("r=%d\n", rate);
    printf("e=1\n");
    printf("p=Image dimensions: %d x %d\n", w, h);

    while (count < sample_count) {
        tmp_pos = (curr_y_pos*w + curr_x_pos)*3;
        if (mflag) {
            a_val = (((image[tmp_pos] +
                       image[tmp_pos + 1] +
                       image[tmp_pos + 2])/3 >> 4) > MID_VAL) ? MAX_VAL : MIN_VAL;
            b_val = 0;
            c_val = 0;
            a_val = (((a_val - MIN_VAL) * (a_max - a_min)) / (MAX_VAL - MIN_VAL)) + a_min;
        } else if (gflag) {
            a_val = (image[tmp_pos] +
                     image[tmp_pos + 1] +
                     image[tmp_pos + 2])/3 >> 4;
            b_val = 0;
            c_val = 0;
            a_val = (((a_val - MIN_VAL) * (a_max - a_min)) / (MAX_VAL - MIN_VAL)) + a_min;
        } else {
            a_val = image[tmp_pos] >> 4;
            b_val = image[tmp_pos+1] >> 4;
            c_val = ((image[tmp_pos+2] >> 4) > MID_VAL) ? 1 : 0;
            a_val = (((a_val - MIN_VAL) * (a_max - a_min)) / (MAX_VAL - MIN_VAL)) + a_min;
            b_val = (((b_val - MIN_VAL) * (b_max - b_min)) / (MAX_VAL - MIN_VAL)) + b_min;
        }

        printf("s=%u,%u,%u,%u,%u,%u\n",
               curr_x_pos + w_off,  curr_y_pos + h_off, a_val, b_val, c_val,1); // x, y, a, b, c, intl_a

        count++;

        //printf("\tx:\t%d\ty:\t%d\t= %d\n", curr_y_pos, curr_x_pos, val);

        if (curr_y_pos & 1) { // Odd row
            if (curr_x_pos == 0) {
                curr_y_pos++;
            } else {
                curr_x_pos--;
            }
        } else { // Even row
            if (curr_x_pos == w-1) {
                curr_y_pos++;
            } else {
                curr_x_pos++;
            }
        }
    }


    free(image);

    printf("f=1\n");
    printf("e=0\n");

    ret = 0;
out_post:
    return ret;
}


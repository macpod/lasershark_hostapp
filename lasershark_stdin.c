/*
lasershark_stdin.c - Main host application to talk with Lasershark devices.
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
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <math.h>
#include <libusb.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif
#include <time.h>
#include "lasersharklib/lasershark_lib.h"
#include "getline_portable.h"
#include "getopt_portable.h"


#define LASERSHARK_VID 0x1fc9
#define LASERSHARK_PID 0x04d8

// Bulk timeout in ms
#define BULK_TIMEOUT 100

int do_exit = 0;


int lasershark_serialnum_len = 64;
unsigned char lasershark_serialnum[64];
uint32_t lasershark_fw_major_version = 0;
uint32_t lasershark_fw_minor_version = 0;


uint32_t lasershark_bulk_packet_sample_count;
uint32_t lasershark_samp_element_count;
uint32_t lasershark_max_ilda_rate;
uint32_t lasershark_dac_min_val;
uint32_t lasershark_dac_max_val;

uint32_t lasershark_ringbuffer_sample_count;

uint32_t lasershark_ilda_rate = 0;

struct libusb_device_handle *ls_devh = NULL;


uint64_t line_number = 0;


struct lasershark_sample
{
    unsigned short a	: 12;
    unsigned short pad	: 2;
    bool c	: 1;
    bool intl_a	: 1;
    unsigned short b	: 16;
    unsigned short x	: 16;
    unsigned short y	: 16;
} __attribute__((packed)) *samples;
uint32_t current_sample_entry = 0;


#ifdef _WIN32
// Handler function will be called on separate thread!
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT: // Ctrl+C
        break;
    case CTRL_BREAK_EVENT: // Ctrl+Break
        break;
    case CTRL_CLOSE_EVENT: // Closing the console window
        break;
    case CTRL_LOGOFF_EVENT: // User logs off. Passed only to services!
        break;
    case CTRL_SHUTDOWN_EVENT: // System is shutting down. Passed only to services!
        break;
    }

    // Return TRUE if handled this message, further handler functions won't be called.
    // Return FALSE to pass this message to further handlers until default handler calls ExitProcess().
    return TRUE;
}

#else

sigset_t mask, oldmask;

static void sig_hdlr(int signum)
{
    switch (signum)
    {
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
#endif

static bool inline send_samples(unsigned int sample_count)
{
    int r, actual;
    do {
        r = libusb_bulk_transfer(ls_devh, (3 | LIBUSB_ENDPOINT_OUT), (unsigned char*)samples,
                                 sizeof(struct lasershark_sample)*sample_count,
                                 &actual, BULK_TIMEOUT);
    } while (!do_exit && r == LIBUSB_ERROR_TIMEOUT);

    if (r < 0 && r != LIBUSB_ERROR_TIMEOUT) {
        fprintf(stderr, "Error sending sample packet: %s", libusb_error_name(r));
        return false;
    }

    return true;
}


// Sample integers are parsed this way vs scanf/etc for speed reasons.
static bool inline parse_sample_integer(char* line, size_t len, unsigned int *pos, unsigned int *val)
{
    unsigned int orig_pos = *pos;
    *val = 0;
    while (*pos < len && line[*pos] >= '0' && line[*pos] <= '9') {
        *val = 10*(*val) + line[*pos]-'0';
        (*pos)++;
        if (*val > lasershark_dac_max_val) {
            return false;
        }
    }

    return orig_pos != *pos;
}


static bool handle_sample(char* line, size_t len)
{
    unsigned int x, y, a, b, c, intl_a;
    unsigned int pos;

    // Lets make a giant if statement for fun
    if (
        len < 14 ||
        (pos = 0, line[pos] != 's') || (pos++, line[pos] != '=') ||
        (pos++, !parse_sample_integer(line, len, &pos, &x)) ||
        pos >= len || line[pos] !=',' ||
        (pos++, !parse_sample_integer(line, len, &pos, &y)) ||
        pos >= len || line[pos] !=',' ||
        (pos++, !parse_sample_integer(line, len, &pos, &a)) ||
        pos >= len || line[pos] !=',' ||
        (pos++, !parse_sample_integer(line, len, &pos, &b)) ||
        pos >= len || line[pos] !=',' ||
        (pos++, pos >= len) || (c=line[pos]-'0', c > 1) ||
        (pos++, pos >= len) || line[pos] !=',' ||
        (pos++, pos >= len) || (intl_a=line[pos]-'0', intl_a > 1)
    ) {
        fprintf(stderr, "Received bad sample command\n");
        return false;
    }

    samples[current_sample_entry].x = x;
    samples[current_sample_entry].y = y;
    samples[current_sample_entry].a = a;
    samples[current_sample_entry].b = b;
    samples[current_sample_entry].c = c;
    samples[current_sample_entry].intl_a = intl_a;

    current_sample_entry++;

    if (current_sample_entry == lasershark_bulk_packet_sample_count) {
        current_sample_entry = 0;
        return send_samples(lasershark_bulk_packet_sample_count);
    }

    return true;
}


static bool handle_set_ilda_rate(char* line, size_t len)
{
    uint32_t rate = 0;
    int rc;
    if (1 != sscanf(line, "r=%u", &rate)) {
        fprintf(stderr, "Received malformated ilda rate command\n");
        return false;
    }

    if (rate == 0 || rate > lasershark_max_ilda_rate) {
        fprintf(stderr, "Received ilda rate outside acceptable range\n");
    }

    lasershark_ilda_rate = rate;
    rc = set_ilda_rate(ls_devh, lasershark_ilda_rate);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "setting ILDA rate failed\n");
        return false;
    }
    printf("Setting ILDA rate worked: %u pps\n", lasershark_ilda_rate);

    return true;
}


static bool handle_set_output(char*line, size_t len)
{
    uint32_t enable = 0;
    int rc;

    if (1 != sscanf(line, "e=%u", &enable)) {
        fprintf(stderr, "Received malfored enable command\n");
        return false;
    }


    rc = set_output(ls_devh, enable ? LASERSHARK_CMD_OUTPUT_ENABLE : LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Setting output failed\n");
        return false;
    }
    if (enable) {
        fprintf(stderr, "Setting output output worked: %u\n", enable);
    }
    return true;
}


static bool handle_print(char* line, size_t len)
{
    bool rc = false;
    if (len > 2 && line[1] == '=') {
        printf("PRINT: %s", line+2);
        rc = true;
    }
    return rc;
}


static bool handle_flush(char* line, size_t len)
{
    int rc;
    uint32_t empty_samples;

    if (current_sample_entry != 0) {
        if (!send_samples(current_sample_entry)) {
            return false;
        }
    }

    current_sample_entry = 0;
    printf("Flushing...\n");
    while (1) {
        rc = get_ringbuffer_empty_sample_count(ls_devh, &empty_samples);
        if (rc != LASERSHARK_CMD_SUCCESS)
        {
            fprintf(stderr, "Getting ringbuffer empty sample count failed.\n");
            return false;
        }

        if (do_exit || empty_samples == lasershark_ringbuffer_sample_count) {
            break;
        }

        // TODO: Use something better than sleep
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        printf("still flushing...\n");
    }

    printf("Flush done\n");
    return true;
}


static bool process_line(char* line, size_t len)
{
    bool rc = true;

    if (len < 2) { // Empty lines are not accepted
        fprintf(stderr, "Empty line encountered on line %" PRIu64 "\n", line_number);
        return false;
    }

    switch(line[0]) {
    case 's':
        rc = handle_sample(line, len);
        break;
    case 'f':
        rc = handle_flush(line, len);
        break;
    case 'r':
        rc = handle_set_ilda_rate(line, len);
        break;
    case 'e':
        rc = handle_set_output(line, len);
        break;
    case 'p':
        rc = handle_print(line, len);
        break;
    case '#': // Comment
        break;
    default:
        fprintf(stderr, "Unknown command received\n");
        rc = false;
    }

    if (!rc) {
        fprintf(stderr, "Error on line %" PRIu64 ": %s", line_number, line);
    }

    line_number++;

    return rc;
}


static void print_lasersharks()
{
    int rc;
    libusb_device **devs;
    struct libusb_device_handle *devh;
    struct libusb_device_descriptor desc;
    ssize_t count;
    ssize_t i;
    unsigned char serial[lasershark_serialnum_len];

    count = libusb_get_device_list(NULL, &devs);

    if (count < 0) {
        fprintf(stderr, "Error encountered acquiring device list: %d\n", (int)count);
        return;
    }

    printf("Connected LaserShark units:\n");
    for (i = 0; i < count; i++) {
        rc = libusb_get_device_descriptor(devs[i], &desc);
        if (rc < 0) {
            fprintf(stderr, "Error obtaining device descriptor: %d\n", /*libusb_error_name(rc)*/rc);
            break;
        }

        if (desc.idVendor == LASERSHARK_VID && desc.idProduct == LASERSHARK_PID) {

            rc = libusb_open(devs[i], &devh);
            if (rc < 0) {
                fprintf(stderr, "Error opening USB device\n");
                break;
            }

            memset(serial, 0, lasershark_serialnum_len);
            rc = libusb_get_string_descriptor_ascii(devh, desc.iSerialNumber, serial, lasershark_serialnum_len);
            if (rc < 0) {
                fprintf(stderr, "Error obtaining iSerialNumber: %d\n", /*libusb_error_name(rc)*/rc);
                break;
            }
            printf("\tiSerialNumber: %s\n", serial);

            libusb_close(devh);
        }
    }

    libusb_free_device_list(devs, 1); // Free the list and dereference all devices
}


static bool open_lasershark(const char* serial)
{
    int rc;
    libusb_device **devs = NULL;
    struct libusb_device_handle *devh = NULL;
    struct libusb_device_descriptor desc;
    ssize_t count;
    ssize_t i;

    count = libusb_get_device_list(NULL, &devs);

    if (count < 0) {
        fprintf(stderr, "Error encountered acquiring device list: %d\n", (int)count);
        return false;
    }

    for (i = 0; i < count; i++) {
        rc = libusb_get_device_descriptor(devs[i], &desc);
        if (rc < 0) {
            fprintf(stderr, "Error obtaining device descriptor: %d\n", /*libusb_error_name(rc)*/rc);
            break;
        }

        if (desc.idVendor == LASERSHARK_VID && desc.idProduct == LASERSHARK_PID) {

            rc = libusb_open(devs[i], &devh);
            if (rc < 0) {
                fprintf(stderr, "Error opening USB device\n");
                break;
            }

            memset(lasershark_serialnum, 0, lasershark_serialnum_len);
            rc = libusb_get_string_descriptor_ascii(devh, desc.iSerialNumber, lasershark_serialnum, lasershark_serialnum_len);
            if (rc < 0) {
                fprintf(stderr, "Error obtaining iSerialNumber: %d\n", /*libusb_error_name(rc)*/rc);
                break;
            }
            if (NULL == serial || strncmp((const char*)lasershark_serialnum, serial, lasershark_serialnum_len)) {
                printf("iSerialNumber: %s\n", lasershark_serialnum);
                break;
            }

            libusb_close(devh);
            memset(lasershark_serialnum, 0, lasershark_serialnum_len);
            devh = NULL;
        }
    }

    libusb_free_device_list(devs, 1); // Free the list and dereference all devices

    if (devh) {
        ls_devh = devh;
        return true;
    }

    return false;
}


void print_help(const char* prog_name, FILE* stream)
{
    fprintf(stream, "%s [OPTION]\n", prog_name);
    fprintf(stream, "\t-h");
    fprintf(stream, "\tPrint this help text\n");
    fprintf(stream, "\t-l");
    fprintf(stream, "\tLists all connected LaserSharks\n");
    fprintf(stream, "\t-s <LaserShark Serial Number>\n");
    fprintf(stream, "\t\tConnect to a specific LaserShark\n");
}


int main (int argc, char *argv[])
{
    int rc;
    uint32_t temp;

    int hflag = 0;
    int lflag = 0;
    int sflag = 0;
    char* requested_serial = NULL;
    int c;

#ifndef _WIN32
    struct sigaction sigact;
#endif

    opterr_portable = 1;
    while (-1 != (c =getopt_portable(argc, argv, "hls:"))) {
        switch(c) {
        case 'h':
            hflag++;
            break;
        case 'l':
            lflag++;
            break;
        case 's':
            sflag++;
            requested_serial = optarg_portable;
            break;
        default:
            print_help(argv[0], stderr);
            exit(1);
        }
    }

    if (lflag && sflag) {
        fprintf(stderr, "Cannot specify both -s and -l flags.\n");
        print_help(argv[0], stderr);
        exit(1);
    }

    if (lflag > 1 || sflag > 1 || hflag > 1) {
        fprintf(stderr, "Cannot specify flags more than once.\n");
        print_help(argv[0], stderr);
        exit(1);
    }

    if (hflag) {
        print_help(argv[0], stdout);
        exit(0);
    }

#ifndef _WIN32
    sigact.sa_handler = sig_hdlr;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);
#endif


    rc = libusb_init(NULL);
    if (rc < 0)
    {
        fprintf(stderr, "Error initializing libusb: %d\n", /*libusb_error_name(rc)*/rc);
        exit(1);
    }

    libusb_set_debug(NULL, 3);

    if (lflag) {
        print_lasersharks();
        goto out_post_release;
    }

    if(!open_lasershark(requested_serial) )
    {
        fprintf(stderr, "Error finding/opening LaserShark\n");
        goto out_post_release;
    }



    rc = libusb_claim_interface(ls_devh, 0);
    if (rc < 0)
    {
        fprintf(stderr, "Error claiming control interface: %d\n", /*libusb_error_name(rc)*/rc);
        goto out_post_release;
    }
    rc = libusb_claim_interface(ls_devh, 1);
    if (rc < 0)
    {
        fprintf(stderr, "Error claiming data interface: %d\n", /*libusb_error_name(rc)*/rc);
        libusb_release_interface(ls_devh, 0);
        goto out_post_release;
    }

    rc = libusb_set_interface_alt_setting(ls_devh, 1, 1);
    if (rc < 0)
    {
        fprintf(stderr, "Error setting alternative (BULK) data interface: %d\n", /*libusb_error_name(rc)*/rc);
        goto out;
    }

    rc = get_fw_major_version(ls_devh, &lasershark_fw_major_version);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting FW Major version failed.\n");
        goto out;
    }
    printf("Getting FW Major version: %d\n", lasershark_fw_major_version);

    rc = get_fw_minor_version(ls_devh, &lasershark_fw_minor_version);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting FW Minor version failed.\n");
        goto out;
    }
    printf("Getting FW Minor version: %d\n", lasershark_fw_minor_version);

    if (lasershark_fw_major_version != LASERSHARK_FW_MAJOR_VERSION ||
            lasershark_fw_minor_version != LASERSHARK_FW_MINOR_VERSION) {
        fprintf(stderr, "Your FW is not capable of proper bulk transfers or clear commands. Please upgrade your firmware.\n");
        goto out;
    }

    printf("Clearing ringbuffer\n");
    rc = clear_ringbuffer(ls_devh);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        fprintf(stderr, "Clearing ringbuffer buffer failed.\n");
        goto out;
    }


    rc = get_bulk_packet_sample_count(ls_devh, &lasershark_bulk_packet_sample_count);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting bulk packet sample count failed\n");
        goto out;
    }
    printf("Getting bulk packet sample count: %d\n", lasershark_bulk_packet_sample_count);

    samples = malloc(sizeof(struct lasershark_sample)*lasershark_bulk_packet_sample_count);
    if (samples == NULL) {
        fprintf(stderr, "Could not allocate sample array.\n");
        goto out;
    }

    rc = get_max_ilda_rate(ls_devh, &lasershark_max_ilda_rate);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting max ilda rate failed\n");
        goto out;
    }
    printf("Getting max ilda rate: %u pps\n", lasershark_max_ilda_rate);


    rc = get_dac_min(ls_devh, &lasershark_dac_min_val);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting dac min failed\n");
        goto out;
    }
    printf("Getting dac min: %d\n", lasershark_dac_min_val);


    rc = get_dac_max(ls_devh, &lasershark_dac_max_val);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        fprintf(stderr, "Getting dac max failed\n");
        goto out;
    }
    printf("getting dac max: %d\n", lasershark_dac_max_val);


    rc = get_ringbuffer_sample_count(ls_devh, &lasershark_ringbuffer_sample_count);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting ringbuffer sample count\n");
        goto out;
    }
    printf("Getting ringbuffer sample count: %d\n", lasershark_ringbuffer_sample_count);


    rc = get_ringbuffer_empty_sample_count(ls_devh, &temp);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting ringbuffer empty sample count failed.\n");
    }
    printf("Getting ringbuffer empty sample count: %d\n", temp);

    rc = set_output(ls_devh, LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Disable output failed\n");
        goto out;
    }
    printf("Disable output worked\n");

    ssize_t read;
    size_t len = 256;

    char *line = malloc(len+1);
    if (line == NULL) {
        fprintf(stderr, "Buffer malloc failed\n");
        goto out;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
#else
    sigemptyset (&mask);
    sigaddset (&mask, SIGUSR1);

    sigprocmask (SIG_BLOCK, &mask, &oldmask);
#endif

    printf("===Running===\n");

    if (-1 == (read = getline_portable(&line, &len, stdin)) || read < 1 || line[0] != 'r' || !process_line(line, read)) {
        fprintf(stderr, "First command did not specify ilda rate. Quitting.\n");
    } else {
        while (!do_exit && -1 != (read = getline_portable(&line, &len, stdin)) && process_line(line, read)) {
            //sigsuspend (&oldmask);
            //printf("Looping... (Must have recieved a signal, don't panic).\n");
        }
        //sigprocmask (SIG_UNBLOCK, &mask, NULL);
    }

    printf("===Ending===\n");
    rc = set_output(ls_devh, LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Disable output failed\n");
        goto out;
    }
    printf("Disable output worked\n");

    rc = get_ringbuffer_empty_sample_count(ls_devh, &temp);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        fprintf(stderr, "Getting ringbuffer empty sample count failed.\n");
    }
    if (lasershark_ringbuffer_sample_count-temp>0 || current_sample_entry) {
        fprintf(stderr, "Warning, not all samples displayed. Consider flushing before quitting.\n");
        fprintf(stderr, "\t%u not sent to Lasershark.\n", current_sample_entry);
        temp = lasershark_ringbuffer_sample_count - temp;
        fprintf(stderr, "\t%u-%u = %u still in Lasershark's buffer.\n", lasershark_ringbuffer_sample_count, temp, lasershark_ringbuffer_sample_count-temp);
    }


    printf("Clearing ringbuffer\n");
    rc = clear_ringbuffer(ls_devh);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        fprintf(stderr, "Clearing ringbuffer buffer failed.\n");
        goto out;
    }


    printf("Quitting gracefully\n");

out:
    libusb_release_interface(ls_devh, 0);
    libusb_release_interface(ls_devh, 1);

out_post_release:
    if (ls_devh)
    {
        libusb_close(ls_devh);
    }
    libusb_exit(NULL);


    return rc;
}


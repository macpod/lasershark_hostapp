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
#include <unistd.h>
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
#include "lasershark_lib.h"
#include "getline_portable.h"


#define LASERSHARK_VIN 0x1fc9
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

struct libusb_device_handle *devh_ctl = NULL;
struct libusb_device_handle *devh_data = NULL;


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
        r = libusb_bulk_transfer(devh_data, (3 | LIBUSB_ENDPOINT_OUT), (unsigned char*)samples,
                                 sizeof(struct lasershark_sample)*sample_count,
                                 &actual, BULK_TIMEOUT);
    } while (!do_exit && r == LIBUSB_ERROR_TIMEOUT);

    if (r < 0 && r != LIBUSB_ERROR_TIMEOUT) {
        printf("Error sending sample packet: %s", libusb_error_name(r));
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
        printf("Received bad sample command\n");
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
        printf("Received malformated ilda rate command\n");
        return false;
    }

    if (rate == 0 || rate > lasershark_max_ilda_rate) {
        printf("Received ilda rate outside acceptable range\n");
    }

    lasershark_ilda_rate = rate;
    rc = set_ilda_rate(devh_ctl, lasershark_ilda_rate);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("setting ILDA rate failed\n");
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
        printf("Received malfored enable command\n");
        return false;
    }


    rc = set_output(devh_ctl, enable ? LASERSHARK_CMD_OUTPUT_ENABLE : LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Setting output failed\n");
        return false;
    }
    if (enable) {
        printf("Setting output output worked: %u\n", enable);
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
        rc = get_ringbuffer_empty_sample_count(devh_ctl, &empty_samples);
        if (rc != LASERSHARK_CMD_SUCCESS)
        {
            printf("Getting ringbuffer empty sample count failed.\n");
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
        printf("Empty line encountered on line %" PRIu64 "\n", line_number);
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
        printf("Unknown command received\n");
        rc = false;
    }

    if (!rc) {
        printf("Error on line %" PRIu64 ": %s", line_number, line);
    }

    line_number++;

    return rc;
}


int main (int argc, char *argv[])
{
    int rc;
    uint32_t temp;

#ifndef _WIN32
    struct sigaction sigact;

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

    devh_ctl = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    devh_data = libusb_open_device_with_vid_pid(NULL, LASERSHARK_VIN, LASERSHARK_PID);
    if (!devh_ctl || !devh_data)
    {
        fprintf(stderr, "Error finding USB device\n");
        goto out_post_release;
    }


    libusb_set_debug(NULL, 3);

    rc = libusb_claim_interface(devh_ctl, 0);
    if (rc < 0)
    {
        fprintf(stderr, "Error claiming control interface: %d\n", /*libusb_error_name(rc)*/rc);
        goto out_post_release;
    }
    rc = libusb_claim_interface(devh_data, 1);
    if (rc < 0)
    {
        fprintf(stderr, "Error claiming data interface: %d\n", /*libusb_error_name(rc)*/rc);
        libusb_release_interface(devh_ctl, 0);
        goto out_post_release;
    }

    rc = libusb_set_interface_alt_setting(devh_data, 1, 1);
    if (rc < 0)
    {
        fprintf(stderr, "Error setting alternative (BULK) data interface: %d\n", /*libusb_error_name(rc)*/rc);
        goto out;
    }


    struct libusb_device_descriptor desc;

    rc = libusb_get_device_descriptor(libusb_get_device(devh_ctl), &desc);
    if (rc < 0) {
        fprintf(stderr, "Error obtaining device descriptor: %d\n", /*libusb_error_name(rc)*/rc);
    }

    memset(lasershark_serialnum, lasershark_serialnum_len, 0);
    rc = libusb_get_string_descriptor_ascii(devh_ctl, desc.iSerialNumber, lasershark_serialnum, lasershark_serialnum_len);
    if (rc < 0) {
        fprintf(stderr, "Error obtaining iSerialNumber: %d\n", /*libusb_error_name(rc)*/rc);
    }
    printf("iSerialNumber: %s\n", lasershark_serialnum);


    rc = get_fw_major_version(devh_ctl, &lasershark_fw_major_version);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting FW Major version failed.\n");
        goto out;
    }
    printf("Getting FW Major version: %d\n", lasershark_fw_major_version);

    rc = get_fw_minor_version(devh_ctl, &lasershark_fw_minor_version);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting FW Minor version failed.\n");
        goto out;
    }
    printf("Getting FW Minor version: %d\n", lasershark_fw_minor_version);

    if (lasershark_fw_major_version != LASERSHARK_FW_MAJOR_VERSION ||
            lasershark_fw_minor_version != LASERSHARK_FW_MINOR_VERSION) {
        printf("Your FW is not capable of proper bulk transfers or clear commands. Please upgrade your firmware.\n");
        goto out;
    }

    printf("Clearing ringbuffer\n");
    rc = clear_ringbuffer(devh_ctl);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        printf("Clearing ringbuffer buffer failed.\n");
        goto out;
    }


    rc = get_bulk_packet_sample_count(devh_ctl, &lasershark_bulk_packet_sample_count);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting bulk packet sample count failed\n");
        goto out;
    }
    printf("Getting bulk packet sample count: %d\n", lasershark_bulk_packet_sample_count);

    samples = malloc(sizeof(struct lasershark_sample)*lasershark_bulk_packet_sample_count);
    if (samples == NULL) {
        printf("Could not allocate sample array.\n");
        goto out;
    }

    rc = get_max_ilda_rate(devh_ctl, &lasershark_max_ilda_rate);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting max ilda rate failed\n");
        goto out;
    }
    printf("Getting max ilda rate: %u pps\n", lasershark_max_ilda_rate);


    rc = get_dac_min(devh_ctl, &lasershark_dac_min_val);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting dac min failed\n");
        goto out;
    }
    printf("Getting dac min: %d\n", lasershark_dac_min_val);


    rc = get_dac_max(devh_ctl, &lasershark_dac_max_val);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        printf("Getting dac max failed\n");
        goto out;
    }
    printf("getting dac max: %d\n", lasershark_dac_max_val);


    rc = get_ringbuffer_sample_count(devh_ctl, &lasershark_ringbuffer_sample_count);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting ringbuffer sample count\n");
        goto out;
    }
    printf("Getting ringbuffer sample count: %d\n", lasershark_ringbuffer_sample_count);


    rc = get_ringbuffer_empty_sample_count(devh_ctl, &temp);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting ringbuffer empty sample count failed.\n");
    }
    printf("Getting ringbuffer empty sample count: %d\n", temp);

    rc = set_output(devh_ctl, LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Disable output failed\n");
        goto out;
    }
    printf("Disable output worked\n");

    ssize_t read;
    size_t len = 256;

    char *line = malloc(len+1);
    if (line == NULL) {
        printf("Buffer malloc failed\n");
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
        printf("First command did not specify ilda rate. Quitting.\n");
    } else {
        while (!do_exit && -1 != (read = getline_portable(&line, &len, stdin)) && process_line(line, read)) {
            //sigsuspend (&oldmask);
            //printf("Looping... (Must have recieved a signal, don't panic).\n");
        }
        //sigprocmask (SIG_UNBLOCK, &mask, NULL);
    }

    printf("===Ending===\n");
    rc = set_output(devh_ctl, LASERSHARK_CMD_OUTPUT_DISABLE);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Disable output failed\n");
        goto out;
    }
    printf("Disable output worked\n");

    rc = get_ringbuffer_empty_sample_count(devh_ctl, &temp);
    if (rc != LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting ringbuffer empty sample count failed.\n");
    }
    if (lasershark_ringbuffer_sample_count-temp>0 || current_sample_entry) {
        printf("Warning, not all samples displayed. Consider flushing before quitting.\n");
        printf("\t%u not sent to Lasershark.\n", current_sample_entry);
        temp = lasershark_ringbuffer_sample_count - temp;
        printf("\t%u-%u = %u still in Lasershark's buffer.\n", lasershark_ringbuffer_sample_count, temp, lasershark_ringbuffer_sample_count-temp);
    }


    printf("Clearing ringbuffer\n");
    rc = clear_ringbuffer(devh_ctl);
    if (rc != LASERSHARK_CMD_SUCCESS) {
        printf("Clearing ringbuffer buffer failed.\n");
        goto out;
    }


    printf("Quitting gracefully\n");

out:
    libusb_release_interface(devh_ctl, 0);
    libusb_release_interface(devh_data, 0);

out_post_release:
    if (devh_ctl)
    {
        libusb_close(devh_ctl);
    }
    if (devh_data)
    {
        libusb_close(devh_data);
    }
    libusb_exit(NULL);


    return rc;
}


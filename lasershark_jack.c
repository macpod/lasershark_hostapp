/*
lasershark_jack.c - Main host application to talk with Lasershark devices.
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
#include <math.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <libusb.h>
#include <signal.h>
#include <time.h>
#include "lasershark_lib.h"


#define LASERSHARK_VIN 0x1fc9
#define LASERSHARK_PID 0x04d8

int do_exit = 0;
pid_t pid;

jack_client_t *client;

typedef jack_default_audio_sample_t sample_t;
typedef jack_nframes_t nframes_t;

jack_port_t *in_x;
jack_port_t *in_y;
jack_port_t *in_g;

nframes_t rate;

// Number of laserjack data packets the ringbuffer will have space for.
#define JACK_RB_PACKETS 256
jack_ringbuffer_t *jack_rb = NULL;
uint32_t jack_rb_len = 0;

uint8_t *laserjack_iso_data_packet_buf = NULL;
int laserjack_iso_data_packet_len = 0;

uint32_t lasershark_packet_sample_count;
uint32_t lasershark_samp_element_count;
uint32_t lasershark_max_ilda_rate;
uint32_t lasershark_dac_min_val;
uint32_t lasershark_dac_max_val;

uint32_t lasershark_ilda_rate = 0;

typedef struct
{
    float x, y, r, g, b;
} bufsample_t;

struct libusb_device_handle *devh_ctl = NULL;
struct libusb_device_handle *devh_data = NULL;
uint32_t max_iso_data_len = 0;


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


void quit_program()
{
    kill(pid, SIGUSR1);
}

/*
Internal callback for cleaning up async writes.
 */
void
WriteAsyncCallback(struct libusb_transfer *transfer)
{
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED || transfer->actual_length != transfer->length)
    {
        printf("ISO transfer err: %d   bytes transferred: %d\n", transfer->status, transfer->actual_length);
    }
    free(transfer->buffer);
    libusb_free_transfer(transfer);
}



/*
Writes an ISO packet to the Lasershark device.
Returns LASERSHARK_CMD_SUCCESS on success, LASERSHARK_CMD_FAIL on failure.
*/
int write_lasershark_data(unsigned char *data, int len)
{
    int rc;

    if (len > max_iso_data_len)
    {
        printf("Oversized iso write length. %d > %d\n", len, max_iso_data_len);
        return LASERSHARK_CMD_FAIL;
    }

    struct libusb_transfer *transfer = libusb_alloc_transfer(1);

    if (!transfer)
    {
        printf("Could not allocate memory for iso transfer.\n");
        return LIBUSB_ERROR_NO_MEM;
    }

    libusb_fill_iso_transfer(transfer, devh_data, (4 | LIBUSB_ENDPOINT_OUT),
                             malloc(len), len, 1, WriteAsyncCallback, 0, 0);

    if (!transfer->buffer)
    {
        printf("Could not create iso transfer packet.\n");
        libusb_free_transfer(transfer);
        return LASERSHARK_CMD_FAIL;
    }

    libusb_set_iso_packet_lengths(transfer, len);

    memcpy(transfer->buffer, data, len);

    rc = libusb_submit_transfer(transfer);

    if(rc != 0)
    {
        printf("Could not submit transfer: rc=%d\n", rc);
        return LASERSHARK_CMD_FAIL;
    }

    return LASERSHARK_CMD_SUCCESS;
}


/*
Converts from one linear scale to another in the sexiest manner possible, with math.
*/
static uint16_t convert(float OldValue, float OldMin, float OldMax, uint32_t NewMax, uint32_t NewMin)
{

    uint16_t val = (uint16_t)((((OldValue - OldMin) * (float)(NewMax - NewMin)) / (OldMax - OldMin)) + (float)NewMin);
    return val;
}


static int process (nframes_t nframes, void *arg)
{
    uint16_t temp[lasershark_samp_element_count];
    int avail, written, i, j, rc;
    nframes_t frm;

    sample_t *i_x = (sample_t *) jack_port_get_buffer (in_x, nframes);
    sample_t *i_y = (sample_t *) jack_port_get_buffer (in_y, nframes);
    sample_t *i_g = (sample_t *) jack_port_get_buffer (in_g, nframes);

    // Read in all samples given to us from the GODLY JACK SERVER
    for (frm = 0; frm < nframes; frm++)
    {
        // Read the data and convert it to a format suitable to be sent out to the lasershark.
        temp[0] = convert(*i_g, -0.0f, 1.0f, lasershark_dac_max_val, lasershark_dac_min_val);
        if (temp[0] >= (lasershark_dac_max_val + lasershark_dac_min_val)/2) {
		temp[0] |= LASERSHARK_C_BITMASK; // If the laser power is >= half the dac output.. turn this ttl channel on.
	}
	temp[0] |= LASERSHARK_INTL_A_BITMASK; // Turn on the interlock pin since this is a valid sample.
        temp[1] = convert(*i_g++, -0.0f, 1.0f, lasershark_dac_max_val, lasershark_dac_min_val);
        temp[2] = convert(*i_x++, -1.0f, 1.0f, lasershark_dac_max_val, lasershark_dac_min_val);
        temp[3] = convert(*i_y++ * -1.0f, -1.0f, 1.0f, lasershark_dac_max_val, lasershark_dac_min_val);
        // Jam the samples in the ringbuffer.
        avail = jack_ringbuffer_write_space(jack_rb);
        if (avail >= lasershark_samp_element_count*sizeof(uint16_t))
        {
            written = jack_ringbuffer_write(jack_rb, (const char*)temp, lasershark_samp_element_count*sizeof(uint16_t));
            if (written != lasershark_samp_element_count*sizeof(uint16_t) )
            {
                printf("Ringbuffer write failure\n");
            }
        }
        else
        {
            printf("Ringbuffer full\n");
            break;
        }
    }

    // Send out as many data packets as we can to the DEMIGOD LASERSHARK DEVICE
    while ((i = jack_ringbuffer_read_space(jack_rb)) >= laserjack_iso_data_packet_len)
    {
        // read from the buffer
        j = jack_ringbuffer_read(jack_rb, (char *)laserjack_iso_data_packet_buf, laserjack_iso_data_packet_len);

        if (j != laserjack_iso_data_packet_len)
        {
            printf("Ringbuffer read failure\n");
            quit_program();
        }

        rc = write_lasershark_data((void *)laserjack_iso_data_packet_buf, laserjack_iso_data_packet_len);
        if (rc != LASERSHARK_CMD_SUCCESS)
        {
            quit_program();
        }
    }

    return 0;
}


static int bufsize (nframes_t nframes, void *arg)
{
    printf ("The maximum buffer size is now %u\n", nframes);
    return 0;
}


static int srate (nframes_t nframes, void *arg)
{
    rate = nframes;
    if (rate > lasershark_max_ilda_rate)
    {
        printf("Rate (%d) is higher than lasershark supports (%d)\n", rate, lasershark_max_ilda_rate);
        quit_program();
        return 1;
    }

    if (lasershark_ilda_rate != 0)
    {
        printf("ILDA rate was already set.. but we were asked to set again.. unimplemented! Dying.\n");
        quit_program();
        return 1;
    }

    lasershark_ilda_rate = nframes;
    printf ("ILDA rate specified as: %u pps\n", lasershark_ilda_rate);


    return 0;
}


static void jack_shutdown (void *arg)
{
    printf("JACK shutting down...\n");
    quit_program();
}


int main (int argc, char *argv[])
{
    int rc;
    struct sigaction sigact;

    char jack_client_name[] = "lasershark";


    pid = getpid();
    sigact.sa_handler = sig_hdlr;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);


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


    max_iso_data_len = libusb_get_max_iso_packet_size(libusb_get_device(devh_data), (4 | LIBUSB_ENDPOINT_OUT));
    printf("Max iso data packet length according to descriptors: %d\n", max_iso_data_len);


    rc = get_samp_element_count(devh_ctl, &lasershark_samp_element_count);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting sample element count: %d\n", lasershark_samp_element_count);
    }
    else
    {
        printf("Getting sample element count failed\n");
        goto out;
    }


    rc = get_packet_sample_count(devh_ctl, &lasershark_packet_sample_count);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting packet sample count: %d\n", lasershark_packet_sample_count);
    }
    else
    {
        printf("Getting packet sample count failed\n");
        goto out;
    }


    rc = get_max_ilda_rate(devh_ctl, &lasershark_max_ilda_rate);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting max ilda rate: %u pps\n", lasershark_max_ilda_rate);
    }
    else
    {
        printf("Getting max ilda rate failed\n");
        goto out;
    }


    rc = get_dac_min(devh_ctl, &lasershark_dac_min_val);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Getting dac min: %d\n", lasershark_dac_min_val);
    }
    else
    {
        printf("Getting dac min failed\n");
        goto out;
    }

    rc = get_dac_max(devh_ctl, &lasershark_dac_max_val);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("getting dac max: %d\n", lasershark_dac_max_val);
    }
    else
    {
        printf("Getting dac max failed\n");
        goto out;
    }


    jack_status_t jack_status;
    jack_options_t  jack_options = JackNullOption;

    if ((client = jack_client_open(jack_client_name, jack_options, &jack_status)) == 0)
    {
        fprintf (stderr, "JACK server not running? FSCK!\n");
        goto out;
    }


    jack_set_process_callback (client, process, 0);
    jack_set_buffer_size_callback (client, bufsize, 0);
    jack_set_sample_rate_callback (client, srate, 0);
    jack_on_shutdown (client, jack_shutdown, 0);

    in_x = jack_port_register (client, "in_x", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    in_y = jack_port_register (client, "in_y", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    in_g = jack_port_register (client, "in_g", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

    if (lasershark_ilda_rate == 0)
    {
        printf("ILDA rate wasn't specified by server.. unimplemented case.. dying\n");
        goto out;
    }

    rc = set_ilda_rate(devh_ctl, lasershark_ilda_rate);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Setting ILDA rate worked: %u pps\n", lasershark_ilda_rate);
    }
    else
    {
        printf("setting ILDA rate failed\n");
        goto out;
    }

    rc = set_output(devh_ctl, LASERSHARK_CMD_OUTPUT_ENABLE);
    if (rc == LASERSHARK_CMD_SUCCESS)
    {
        printf("Enable output worked\n");
    }
    else
    {
        printf("Enable output failed\n");
        goto out;
    }

    laserjack_iso_data_packet_len = lasershark_packet_sample_count * lasershark_samp_element_count * sizeof(uint16_t);
    laserjack_iso_data_packet_buf = malloc(laserjack_iso_data_packet_len);
    if (laserjack_iso_data_packet_buf == NULL)
    {
        printf("Could not allocate laserjack iso data packet buffer\n");
        goto out;
    }

    jack_rb_len = laserjack_iso_data_packet_len * JACK_RB_PACKETS;
    jack_rb = jack_ringbuffer_create(jack_rb_len);
    if (jack_rb == NULL)
    {
        printf("Could not allocate JACK ringbuffer\n");
        goto out;
    }


    // lock the buffer into memory, this is *NOT* realtime safe, do it before
    // using the buffer!
    rc = jack_ringbuffer_mlock(jack_rb);
    if (rc)
    {
        printf("Could not lock JACK ringbuffer memory\n");
        goto out;
    }

    if (jack_activate (client))
    {
        fprintf (stderr, "Cannot activate JACK client");
        goto out;
    }


    sigemptyset (&mask);
    sigaddset (&mask, SIGUSR1);

    sigprocmask (SIG_BLOCK, &mask, &oldmask);


    printf("Running\n");
    do
    {
	libusb_handle_events(0); // Quick hack.. this should be made better later.
        //sigsuspend (&oldmask);
        //printf("Looping... (Must have recieved a signal, don't panic).\n");
    }
    while (!do_exit);
    //sigprocmask (SIG_UNBLOCK, &mask, NULL);


    printf("Quitting gracefully\n");

// THINGS COME HERE TO DIE!!!!!!!!!!!!!!!!!!!
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

    if (jack_rb != NULL)
    {
        jack_ringbuffer_free(jack_rb);
    }

    if (laserjack_iso_data_packet_buf != NULL)
    {
        free(laserjack_iso_data_packet_buf);
    }


    return rc;
}


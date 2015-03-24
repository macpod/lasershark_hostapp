CC=$(CROSS)gcc
LD=$(CROSS)ld
AR=$(CROSS)ar
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-Wall 

all: lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_twostep
lasershark_jack: lasershark_jack.c lasershark_lib.c
	$(CC) $(CFLAGS) -o lasershark_jack lasershark_jack.c lasershark_lib.c `$(PKG_CONFIG) --libs --cflags jack libusb-1.0`

lasershark_stdin: lasershark_stdin.c lasershark_lib.c
	$(CC) $(CFLAGS) -o lasershark_stdin lasershark_stdin.c lasershark_lib.c getline_portable.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

lasershark_stdin_circlemaker: lasershark_stdin_circlemaker.c
	$(CC) $(CFLAGS) -o lasershark_stdin_circlemaker lasershark_stdin_circlemaker.c -lm

lasershark_twostep: lasershark_twostep.c lasershark_uart_bridge_lib.c ls_ub_twostep_lib.c twostep_host_lib.c twostep_common_lib.c
	$(CC) $(CFLAGS) -o lasershark_twostep lasershark_twostep.c lasershark_uart_bridge_lib.c ls_ub_twostep_lib.c twostep_host_lib.c twostep_common_lib.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

clean:
	rm -f lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_twostep

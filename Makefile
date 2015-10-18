# Notes:
#   MXE was used to cross compile programs for windows
#   make CROSS=i686-w64-mingw32.static- all-windows
#   make CROSS=x86_64-w64-mingw32.static- all-windows

CC=$(CROSS)gcc
LD=$(CROSS)ld
AR=$(CROSS)ar
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-Wall

all: lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_twostep

all-windows: lasershark_stdin-windows lasershark_stdin_circlemaker-windows

lasershark_jack: lasershark_jack.c lasershark_lib.c lasershark_lib.h
	$(CC) $(CFLAGS) -o lasershark_jack lasershark_jack.c lasershark_lib.c `$(PKG_CONFIG) --libs --cflags jack libusb-1.0`

lasershark_stdin-windows: CFLAGS+= -mno-ms-bitfields
lasershark_stdin-windows: lasershark_stdin
lasershark_stdin: lasershark_stdin.c lasershark_lib.c lasershark_lib.h \
                    getline_portable.c getline_portable.h getopt_portable.c getopt_portable.h
	$(CC) $(CFLAGS) -o lasershark_stdin lasershark_stdin.c lasershark_lib.c \
                        getline_portable.c getopt_portable.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

lasershark_stdin_circlemaker-windows: CFLAGS+= -mno-ms-bitfields
lasershark_stdin_circlemaker-windows: lasershark_stdin_circlemaker
lasershark_stdin_circlemaker: lasershark_stdin_circlemaker.c
	$(CC) $(CFLAGS) -o lasershark_stdin_circlemaker lasershark_stdin_circlemaker.c -lm

lasershark_twostep: lasershark_twostep.c lasershark_uart_bridge_lib.c lasershark_uart_bridge_lib.h \
                        ls_ub_twostep_lib.c ls_ub_twostep_lib.h \
                        twostep_host_lib.c twostep_host_lib.h twostep_common_lib.c twostep_common_lib.h
	$(CC) $(CFLAGS) -o lasershark_twostep lasershark_twostep.c lasershark_uart_bridge_lib.c \
                        ls_ub_twostep_lib.c twostep_host_lib.c twostep_common_lib.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

clean:
	rm -f lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_twostep

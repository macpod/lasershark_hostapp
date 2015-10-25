# Notes:
#   MXE was used to cross compile programs for windows
#   make CROSS=i686-w64-mingw32.static- all-windows
#   make CROSS=x86_64-w64-mingw32.static- all-windows

CC=$(CROSS)gcc
CPP=$(CROSS)g++
LD=$(CROSS)ld
AR=$(CROSS)ar
PKG_CONFIG=$(CROSS)pkg-config
CFLAGS=-Wall

all: lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_stdin_displayimage lasershark_twostep

all-windows: lasershark_stdin-windows lasershark_stdin_circlemaker-windows lasershark_stdin_displayimage-windows

lasershark_jack: lasershark_jack.c lasersharklib/lasershark_lib.c lasersharklib/lasershark_lib.h
	$(CC) $(CFLAGS) -o lasershark_jack lasershark_jack.c lasersharklib/lasershark_lib.c `$(PKG_CONFIG) --libs --cflags jack libusb-1.0`

lasershark_stdin-windows: CFLAGS+= -mno-ms-bitfields
lasershark_stdin-windows: lasershark_stdin
lasershark_stdin: lasershark_stdin.c lasersharklib/lasershark_lib.c lasersharklib/lasershark_lib.h \
                    getline_portable.c getline_portable.h getopt_portable.c getopt_portable.h
	$(CC) $(CFLAGS) -o lasershark_stdin lasershark_stdin.c lasersharklib/lasershark_lib.c \
                        getline_portable.c getopt_portable.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

lasershark_stdin_circlemaker-windows: CFLAGS+= -mno-ms-bitfields
lasershark_stdin_circlemaker-windows: lasershark_stdin_circlemaker
lasershark_stdin_circlemaker: lasershark_stdin_circlemaker.c
	$(CC) $(CFLAGS) -o lasershark_stdin_circlemaker lasershark_stdin_circlemaker.c -lm

lasershark_stdin_displayimage-windows: CFLAGS+= -mno-ms-bitfields
lasershark_stdin_displayimage-windows: lasershark_stdin_displayimage
lasershark_stdin_displayimage: lasershark_stdin_displayimage.c getopt_portable.c getopt_portable.h lodepng/lodepng.cpp lodepng/lodepng.h
	$(CC) $(CFLAGS) -o lasershark_stdin_displayimage lasershark_stdin_displayimage.c -x c lodepng/lodepng.cpp -x none getopt_portable.c

lasershark_twostep: lasershark_twostep.c lasersharklib/lasershark_uart_bridge_lib.c lasersharklib/lasershark_uart_bridge_lib.h \
                        twosteplib/ls_ub_twostep_lib.c twosteplib/ls_ub_twostep_lib.h \
                        twosteplib/twostep_host_lib.c twosteplib/twostep_host_lib.h \
                        twosteplib/twostep_common_lib.c twosteplib/twostep_common_lib.h
	$(CC) $(CFLAGS) -o lasershark_twostep lasershark_twostep.c lasersharklib/lasershark_uart_bridge_lib.c \
                        twosteplib/ls_ub_twostep_lib.c twosteplib/twostep_host_lib.c \
                        twosteplib/twostep_common_lib.c `$(PKG_CONFIG) --libs --cflags libusb-1.0`

clean:
	rm -f  *.o lasershark_jack lasershark_stdin lasershark_stdin_circlemaker lasershark_stdin_displayimage lasershark_twostep

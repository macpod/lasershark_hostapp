all: lasershark_jack lasershark_twostep
lasershark_jack: lasershark_jack.c lasershark_lib.c
	gcc -Wall -o lasershark_jack lasershark_jack.c lasershark_lib.c `pkg-config --libs --cflags jack libusb-1.0`

lasershark_twostep: lasershark_twostep.c lasershark_uart_bridge_lib.c ls_ub_twostep_lib.c twostep_host_lib.c twostep_common_lib.c
	gcc -Wall -o lasershark_twostep lasershark_twostep.c lasershark_uart_bridge_lib.c ls_ub_twostep_lib.c twostep_host_lib.c twostep_common_lib.c `pkg-config --libs --cflags libusb-1.0`

clean:
	rm lasershark_jack lasershark_twostep

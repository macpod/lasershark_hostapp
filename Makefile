lasershark_jack: lasershark_jack.c lasershark_lib.c
	gcc -Wall -o lasershark_jack lasershark_jack.c lasershark_lib.c `pkg-config --libs --cflags jack libusb-1.0`


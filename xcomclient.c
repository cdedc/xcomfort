#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include "xcomfort.h"

int main(int argc, char *argv[]) {
	libusb_device_handle *dev;

	lxc_init();
	dev = lxc_open();
	if (!dev) { 
		printf("Cant find / claim / open USB device.\n");
		exit(EXIT_FAILURE); 		// FAILURE
	}
	// lxc_get_stats(dev);			// get some status from the USB gateway itself
	lxc_assign_sensor(dev, 8);		// simulate a BASIC MODE assignment of a sensor to a datapoint
	while (1) lxc_poll_rx(dev);		// then receive any messages that fly around and log them to the screen until ^c
	lxc_close(dev);
	lxc_exit();

	exit(0);				// SUCCESS
}

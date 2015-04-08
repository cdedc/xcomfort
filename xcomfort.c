// #define LXC_DEBUG		// debug (verbose) tracing to stdout
// #define LXC_INFO		// info (less verbose) tracing to stdout
// or comment both for no output

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>
#include "xcomfort.h"

static libusb_context *ctx = NULL; //a libusb session

static void udelay(int i) {
	int j;
	for (j = 0; j < i; j++) ;
}

static void init_packet(union lxc_packet *x) {
	int i;
	for (i = 0; i < LXC_MAX_PACKET_SIZE; i++) x->raw.rawdata[i] = 0x00;
}


static int hexdump(const char *data, size_t len) {
	size_t i;
	for (i = 0; i < len; i++) {
		if (i > 0) printf(" ");
		if ((i % 8) == 0) printf("\n");
		printf("%02X", data[i] & 0xff);
	}
	printf("\n");
	return 1;
}

void lxc_init() {
	int r = libusb_init(&ctx);
	PD("lxc_init GO\n");
	if (r < 0) {
		fprintf(stderr, "libusb init error\n");
		exit(1);
	}
	PD("lxc_init OK\n");
}


void lxc_exit() {
	PD("libusb exit GO\n");
	libusb_exit(ctx);
	PD("libusb exit OK\n");
}


void lxc_close(libusb_device_handle *dev) {
	PD("lxc_close GO\n");
	libusb_release_interface(dev, LXC_USB_INTERFACE);
	PD("  libusb release interface OK\n");
	libusb_close(dev);
	PD("  libusb close OK\n");
	PD("lxc_close OK\n");
}


libusb_device_handle *lxc_open() {
	// handles all device opening functions needed in one function
	struct libusb_device_handle *dev;

	PD("lxc_open GO\n");

	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in some documentation
	PD("  lxc_open debug level 3 set\n");

	// open the device by directly asking for "EATON / USB-Communication Gateway"
	dev = libusb_open_device_with_vid_pid(ctx, LXC_USB_VENDOR, LXC_USB_PRODUCT);

	if (dev == NULL) {
		fprintf(stderr, "  >> Failed to find/open USB device\n");
		return dev;
	}
	PD2("libusb_open successful, USB device %04x:%04x found\n", LXC_USB_VENDOR, LXC_USB_PRODUCT);
	
	PD("  checking for kernel driver active (device already in use?)\n");
	int ret = libusb_kernel_driver_active(dev, LXC_USB_INTERFACE);
	if (ret == 1) {
		ret = libusb_detach_kernel_driver(dev, LXC_USB_INTERFACE);
		if (ret != 0) {
			printf("  >> Can't detect device's kernel driver %d\n", ret);
			libusb_close(dev);
			return NULL;
		}
		PD("  device contacted\n");
	}
	PD("  device is not in use\n");
	
	ret = libusb_set_configuration(dev, 1);
	if (ret != 0) {
		printf("  >> Can't set configuration on USB device: %d\n", ret);
		libusb_close(dev);
		return NULL;
	}
	PD("  device is now configured\n");
	
	ret = libusb_claim_interface(dev, LXC_USB_INTERFACE);
	if (ret != 0) {
		printf("  >> Could not claim interface: %d\n", ret);
		libusb_close(dev);
		return NULL;
	}
	PD("  device is now claimed\n");

	PD("lxc_open OK\n");
	return dev;
}


void 	lxc_get_stats(struct libusb_device_handle *dev) {
	union lxc_packet req;
	union lxc_packet rsp;
	int ret;
	int try;

	// enumerate several statistics from the USB interface itself
	
	// up to now I have NO idea why you have to send a cfg-request several times before you get an answer
	// but from my current testing I found that after max 5 tries you will get an answer
	for (try = 1; try < 5; try++) {
		init_packet(&req);
		req.cfg.len = 0x04;
		req.cfg.type = LXC_PT_CFG;
		req.cfg.cf_type = LXC_CT_SERIAL;
		req.cfg.cf_mode = 0x00;
		lxc_out(dev, &req);
		init_packet(&rsp);
		if (lxc_in(dev, &rsp, 100) == 0) {		// 100ms max waiting time
			// got 32-bit Serial No.
			printf("Serial number data:\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, serialno = 0x%08X / %08d\n", 
			 	rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data, rsp.sta.st_data)
			;
			break;
		} else {
			printf("IN: no reply.\n");
		}
	}

	for (try = 1; try < 5; try++) {
		init_packet(&req);
		req.cfg.len = 0x04;
		req.cfg.type = LXC_PT_CFG;
		req.cfg.cf_type = LXC_CT_RELEASE;
		req.cfg.cf_mode = 0x00; // ask for release numbers
		lxc_out(dev, &req);
		init_packet(&rsp);
		if (lxc_in(dev, &rsp, 100) == 0) {
			// got 32-bit Serial No.
			printf("Release number data:\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, release = 0x%08X\n", 
				rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data)
			;
			break;
		} else {
			printf("IN: no reply.\n");
		}
	}

	for (try = 1; try < 5; try++) {
		init_packet(&req);
		req.cfg.len = 0x04;
		req.cfg.type = LXC_PT_CFG;
		req.cfg.cf_type = LXC_CT_RELEASE;
		req.cfg.cf_mode = 0x10; // ask for revision numbers
		lxc_out(dev, &req);
		init_packet(&rsp);
		if (lxc_in(dev, &rsp, 100) == 0) {
			// got 32-bit Serial No.
			printf("Release number data:\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, revision = 0x%08X\n", 
				rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data)
			;
			break;
		} else {
			printf("IN: no reply.\n");
		}
	}

	// TIME ACCOUNT
	for (try = 1; try < 5; try++) {
		init_packet(&req);
		req.cfg.len = 0x04;
		req.cfg.type = LXC_PT_CFG;
		req.cfg.cf_type = LXC_CT_TIMEACCOUNT;
		req.cfg.cf_mode = 0x00; // ask for status
		lxc_out(dev, &req);
		init_packet(&rsp);
		if (lxc_in(dev, &rsp, 100) == 0) {
			// got 32-bit Serial No.
			printf("Release number data:\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, time = %d%\n", 
				rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data)
			;
			break;
		} else {
			printf("IN: no reply.\n");
		}
	}
}


int	lxc_assign_sensor(struct libusb_device_handle *dev, int dp) {
	// simulate an BASIC MODE assignment of a sensor with DP dp and wait for the gateway's reply
	// the following protocol is used:
	//
	// host  ==> TX  (0xB1) "go into learn mode simulation" ==> USBGW
	// USBGW ==> STA (0xC3) "ACK learn mode simulation is now on" ==> host (normally "1C 04" == MRF OK)
	// USBGW ==> RX  (0xC1) "button-press on a remote sensor was detected and is now assigned to datapoint dp" ==> host 
	// USBGW ==> STA (0xC3) "ACK learn mode simulation is completed" ==> host (normally "1C 04" == MRF OK)
	union lxc_packet cmd;
	union lxc_packet rsp;
	int try;

	for (try = 1; try < 5; try++) {
		init_packet(&cmd);
		cmd.tx.len = 0x06;		// cmd is always 6 bytes
		cmd.tx.type = LXC_PT_TX;	// 0xB1
		cmd.tx.tx_datapoint = dp;
		cmd.tx.tx_event = 0x80; 	// "basic mode command"
		cmd.tx.tx_event_data = 0x01; 	// "Learn mode on"
		cmd.tx.tx_priority = 0x00;	// "prio highest"
		lxc_out(dev, &cmd);
		init_packet(&rsp);
		printf("Learn Mode Assignment status: "); fflush(stdout);
		if (lxc_in(dev, &rsp, 100) == 0) {
			// should get an ACK imediately
			printf("\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, rest = %d%\n", 
				rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data)
			;
			break;
		} else {
			printf("No reply.\n");
		}
	}

	printf("PRESS A BUTTON ON THE SENSOR NOW!..."); fflush(stdout);

	// now get back the data from the sensor: LXC_PT_RX (0xC1)
	init_packet(&rsp);
	printf("Learn Mode RX data: "); fflush(stdout);
	if (lxc_in(dev, &rsp, 0) == 0) {
		// should get an ACK imediately
		printf("\n  len 0x%02X, type = 0x%02X, datapoint = %02d, msg_type = 0x%02X\n  data_type = 0x%02X info_short = 0x%02X, data = %08d, sigquali = %0d%, battery = %3d%\n", 
			rsp.rx.len, rsp.rx.type, rsp.rx.rx_datapoint, rsp.rx.rx_msg_type, rsp.rx.rx_data_type, 
			rsp.rx.rx_info_short, rsp.rx.rx_data, rsp.rx.rx_rssi, rsp.rx.rx_battery)
		;
	} else {
		printf("No reply.\n");
	}
	
	// get back the hopefully "OK" ACK from the sensor: LXC_PT_STA (0xC3)
	init_packet(&rsp);
	printf("Gateway status data: "); fflush(stdout);
	if (lxc_in(dev, &rsp, 0) == 0) {
		// should get an STATUS imediately
		printf("\n  len %d, type = 0x%02X, st_type = 0x%02X, st_status = 0x%02X, rest = %d%\n", 
			rsp.sta.len, rsp.sta.type, rsp.sta.st_type, rsp.sta.st_status, rsp.sta.st_data)
		;
	} else {
		printf("No reply.\n");
	}

}


int	lxc_poll_rx(struct libusb_device_handle *dev) {
	// polling the USB gateway for RX packets, using a 1 second polling interval
	// this should probably better be done in a background process?
	union lxc_packet rsp;

	// get back ANY LXC_PT_RX (0xC1) packet that is in the queue
	init_packet(&rsp);
	printf("Gateway RX data: "); fflush(stdout);
	if (lxc_in(dev, &rsp, 1000) == 0) {
		printf("\n  len 0x%02X, type = 0x%02X, datapoint = %02d, msg_type = 0x%02X\n  data_type = 0x%02X info_short = 0x%02X, data = %08d, sigquali = %0d%, battery = %3d%\n", 
			rsp.rx.len, rsp.rx.type, rsp.rx.rx_datapoint, rsp.rx.rx_msg_type, rsp.rx.rx_data_type, 
			rsp.rx.rx_info_short, rsp.rx.rx_data, rsp.rx.rx_rssi, rsp.rx.rx_battery)
		;
	} else {
		printf("No packet waiting.\n");
	}
}


int 	lxc_out(struct libusb_device_handle *dev, union lxc_packet *request) {
	// just send one request not waiting for any response
	int ret;
	int i;
	int transferred;

	PI("lxc_out GO\n");
	PI("  COMMAND => ");
	PI1("", hexdump(request->raw.rawdata, sizeof(union lxc_packet)));

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_OUT, request->raw.rawdata, LXC_MAX_PACKET_SIZE, &transferred, LXC_SEND_TIMEOUT);
	if (ret < 0) {
		PI2("    >> Interrupt write error %d aka %s, ", ret, libusb_error_name(ret));
		PI1("DATA (len = %d) => ", transferred); 
		PI1("", hexdump(request->raw.rawdata, transferred));
	}

	PI("lxc_out OK\n"); 
	return ret;
}


int 	lxc_in(struct libusb_device_handle *dev, union lxc_packet *response, int timeout) {
	// do one-time response polling with timeout
	int ret;
	int transferred;

	PI("lxc_in GO\n");
	init_packet(response);
	// PI1("zeroed packet\n", hexdump(response->raw.rawdata, sizeof(union lxc_packet)));
	transferred = 0;

	ret = libusb_interrupt_transfer(dev, LXC_USB_ENDPOINT_IN, response->raw.rawdata, LXC_MAX_PACKET_SIZE, &transferred, timeout);
	if (ret < 0) {
		PI2("    >> Interrupt read error %d aka %s, ", ret, libusb_error_name(ret));
		PI1("DATA (len = %d) => ", transferred); 
		PI1("", hexdump(response->raw.rawdata, transferred));
	}

	PI("lxc_io OK\n");
	return ret;
}

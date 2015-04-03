#ifndef LIBXCOMFORT_H
#define LIBXCOMFORT_H

#include <libusb-1.0/libusb.h>

#define LXC_MAX_PACKET_SIZE 0x20		// important, set this to 0x20 as "lsusb -v" says: "wMaxPacketSize     0x0020  1x 32 bytes"
#define LXC_SEND_TIMEOUT 1000
#define LXC_RECV_TIMEOUT 3000
#define LXC_USB_VENDOR 0x188a 			// reserved value for "Moeller"
#define LXC_USB_PRODUCT 0x1101 			// reserved value for "USB gateway interface"
#define LXC_USB_INTERFACE 0			// default 
#define LXC_USB_ENDPOINT_IN  0x81 		// EP 1 IN, you can see this with "lsusb -v"
#define LXC_USB_ENDPOINT_OUT 0x02 		// EP 2 OUT, you can see this with "lsusb -v"


enum lxc_pkt_type {					// the available LXC (MGW) packet types
	LXC_PT_TX = 					0xB1,
	LXC_PT_CFG =					0xB2,
	LXC_PT_RX = 					0xC1,
	LXC_PT_STA = 					0xC3
};

enum lxc_opcode {					// MGW_PT_TX 0xb1 commands 
	LXC_OPCODE_ON_OFF = 				0x0a, // value = 0 means off, 1 means on
	LXC_OPCODE_DIM_GET = 				0x0b, // Get current value from actuator
	LXC_OPCODE_DIM_SET = 				0x0c, // Percent value 0-100
	LXC_OPCODE_DIM_GRADUAL_STOP = 			0x0d, // aka OFF BUTTON RELEASE (works for stopping both directions)
	LXC_OPCODE_DIM_GRADUAL_START = 			0x0e, // aka OFF/ON BUTTON HOLD: Start gradual dim up or down, value = 0 is down/off, 1 is up/on
	LXC_OPCODE_ON_OFF2 = 				0x0f, // seems identical to LXC_OPCODE_ON_OFF.
	LXC_OPCODE_SEND_MEASUREMENT_VALUE = 		0x11,
	LXC_OPCODE_SEND_MEASUREMENT_VALUE2 = 		0x1a,
	LXC_OPCODE_UNKNOWN1 = 				0x2a,
	LXC_OPCODE_UNKNOWN2 = 				0x2b,
	LXC_OPCODE_SEND_TEMPERATURE = 			0x2c, // Some kind of temperature
	LXC_OPCODE_SEND_MEASUREMENT_VALUE3 = 		0x30, // Has zero decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE4 = 		0x31, // Has one decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE5 = 		0x32, // Has two decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE6 = 		0x33, // Has three decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE7 = 		0x40, // Has zero decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE8 = 		0x41, // Has one decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE9 = 		0x42, // Has two decimals?
	LXC_OPCODE_SEND_MEASUREMENT_VALUE10 = 		0x43, // Has three decimals?
	LXC_OPCODE_SEND_TEMPERATURE2 = 			0x44,
	LXC_OPCODE_SEND_TEMPERATURE3 = 			0x45,
	LXC_TE_BASIC_MODE = 				0x80,	//  basic mode special commands
};

enum lxc_statcode {					// MGW_PT_ACK 0xC3, responses to MGW_PT_CFG 0xB2
	LXC_STT_CONNEX = 				0x02,
	LXC_STT_ERROR = 				0x09,
	LXC_STT_TIMEACCOUNT =				0x0A,
	LXC_STT_SEND_OK_MRF = 				0x0D,
	LXC_STT_SERIAL = 				0x0E,
	LXC_STT_RELEASE = 				0x1B,
//	LXC_STT_SEND_OK_MRF =				0x04
};


union lxc_packet {
	struct {		// RAW DATA, using struct "raw" here for conformance
	uint8_t rawdata[LXC_MAX_PACKET_SIZE];
	} raw;

	struct {		// LXC_PT_TX packet 0xB1, this is the basic "command packet", it simulates a sensors (ligth switch / dimmer / etc) output
	uint8_t len; 		// Seems it should always be 6 (or 7 or 9, if 2 / 4 byte event_data is in use)
	uint8_t type; 		// LXC_PT_TX
	uint8_t tx_datapoint;	// Data point (1-99) of destination of radiogram
	uint8_t tx_event; 	// 
	uint8_t tx_event_data;	// Can be 1 or 2 or 4 bytes! for the cmds usere here it is OK to have just 1 byte
	uint8_t tx_priority; 	// 0x00 = high
	} tx;

	struct {		// LXC_PT_CFG packet 0xB2, this packet type is for configuration / management purposes
	uint8_t len; 		// always 4
	uint8_t type; 		// LXC_PT_CFG
	uint8_t cf_type;	// 
	uint8_t cf_mode;	//
	} cfg;

	struct {		// LXC_PT_RX packet 0xC1, this is sent asynchronously by the USB gateway when the gateways reads radio data from sensors (also actors?)
	uint8_t len; 		// always 12
	uint8_t type; 		// LXC_PT_RX
	uint8_t rx_datapoint;	// Data point (1-99) of origin of radiogram
	uint8_t rx_msg_type; 	// 
	uint8_t rx_data_type;	// the type of rx_data
	uint8_t rx_info_short;	//
	uint32_t rx_data;	// Complete data value, format according to DATA_TYPE, High byte first
	uint8_t rx_rssi;	// Received Signal Strength Indication: < 50 very good; > 90 very bad
	uint8_t rx_battery; 	// Current Battery Status
	} rx;

	struct {		// LXC_PT_STA packet 0xC3, this is the immediate answer to any LXC_PT_CFG packet 0xB2
	uint8_t len; 		// the USB gateway will always set this to 8
	uint8_t type; 		// the USB gateway will always set this to LXC_PT_ACK = 0xC3
	uint8_t st_type;	// this repeats the LXC_PT_CFG packet cf_type
	uint8_t st_status;	// this repeats the LXC_PT_CFG packet cf_mode
	uint32_t st_data; 	// Used only sometimes, else 0
	} sta;
};

// init USB functions
libusb_device_handle 	*lxc_open();
void 			lxc_close(libusb_device_handle *dev);
void 			lxc_init(void);
void 			lxc_exit(void);
// compount functions
void 			lxc_get_stats(struct libusb_device_handle *dev);
int			lxc_assign_sensor(struct libusb_device_handle *dev, int dp);
// basic functions
int 			lxc_io(struct libusb_device_handle *dev, union lxc_packet *request, union lxc_packet *response);
int 			lxc_out(struct libusb_device_handle *dev, union lxc_packet *request);
int 			lxc_in(struct libusb_device_handle *dev, union lxc_packet *response, int timeout);

///// DEBUG flags

#ifdef LXC_DEBUG
#define PD(X) { printf(X); fflush(stdout); }
#else
#define PD(X)
#endif

#ifdef LXC_DEBUG
#define PD1(X,Y) { printf(X,Y); fflush(stdout); }
#else
#define PD1(X,Y)
#endif

#ifdef LXC_DEBUG
#define PD2(X,Y,Z) { printf(X,Y,Z); fflush(stdout); }
#else
#define PD2(X,Y,Z)
#endif

///// INFO

#ifdef LXC_INFO
#define PI(X) { printf(X); fflush(stdout); }
#else
#define PI(X)
#endif

#ifdef LXC_INFO
#define PI1(X,Y) { printf(X,Y); fflush(stdout); }
#else
#define PI1(X,Y)
#endif

#ifdef LXC_INFO
#define PI2(X,Y,Z) { printf(X,Y,Z); fflush(stdout); }
#else
#define PI2(X,Y,Z)
#endif

#endif

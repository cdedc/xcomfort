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


// the 4 core packet types used by the USB gateway
enum lxc_pkt_type {					// the available LXC (MGW) packet types
	LXC_PT_TX = 					0xB1,
	LXC_PT_CFG =					0xB2,
	LXC_PT_RX = 					0xC1,
	LXC_PT_STA = 					0xC3
};


// some of the more often used field codes
enum lxc_tx_event {					// for use in MGW_PT_TX (0xb1) -> tx_events 
							// main commands
	LXC_TE_SWITCH = 				0x0a, // value = 0 means off, 1 means on
	LXC_TE_DIM =	 				0x0d, // dimming, analogue
	LXC_TE_JALO =	 				0x0e, // jalousie
							// aux commands
	LXC_TE_PUSHBUTTON =				0x50, // simulates all types of pushbuttons
	LXC_TE_REQUEST = 				0x0B, // request for LXC_RMT_STATUS (room manager)
	LXC_TE_BASIC = 					0x80, // BASIC MODE configuration via USB gateway
};


enum lxc_tx_event_data {				// 
							// command data for switching (tx_event = LXC_TE_SWITCH)
	LXC_TED_OFF = 					0x00, // switch off
	LXC_TED_ON = 					0x01, // switch on
							// command data for dimming (tx_event = LXC_TE_DIM)
	LXC_TED_STOP = 					0x00, // stop dimming change (keep current dim value)
	LXC_TED_DARKER = 				0x04, // set darker (dim softly darker)
	LXC_TED_BRIGHTER = 				0x0F, // set brighter (dim softly brighter)
							// command data for jalousie (tx_event = LXC_TE_JALO)
	LXC_TED_CLOSE = 				0x00, // 
	LXC_TED_OPEN = 					0x01, // 
	LXC_TED_JSTOP = 				0x02, // 
	LXC_TED_STEP_CLOSE = 				0x10, // 
	LXC_TED_STEP_OPEN = 				0x11, // 
							// pushbutton (simulation) commands
	LXC_TED_UP = 					0x50, // short press on UP button
	LXC_TED_DOWN = 					0x51, // short press on DOWN button
	LXC_TED_UP_PRESSED = 				0x54, // long press on UP button
	LXC_TED_UP_RELEASED = 				0x55, // stopped pressing UP button
	LXC_TED_DOWN_PRESSED = 				0x56, // long press on DOWN button
	LXC_TED_DOWN_RELEASED = 			0x57, // stopped pressing DOWN button
							// BASIC MODE configuration
	LXC_TED_LEARNMODE_ON =				0x01, //
	LXC_TED_LEARNMODE_OFF =				0x00, //
	LXC_TED_ASSIGN_ACTUATOR = 			0x10, //
	LXC_TED_REMOVE_ACTUATOR = 			0x20, //
	LXC_TED_REMOVE_SENSORS = 			0x30, // mass remove a sensor from all connections
							// set dimming value directly, this cant be doine using sensors or real pushbuttons,
							// this feature can only be used via the USB gateway
	LXC_TED_PERCENT = 				0x40, // next byte in the packet is the dimming value in % (0x00 .. 0x64)
};


enum lxc_tx_priority {					// high nibble (highest 4 bits) may be used by the USB gateway programmer
							// low nibble is:
	LXC_TP_DEFAULT =				0x00, // default priority = identical LXC_TP_STANDARD
	LXC_TP_LOWEST =					0x01, // 
	LXC_TP_LOW =					0x02, // 
	LXC_TP_STANDARD =				0x03, // 
	LXC_TP_HIGH =					0x04, // 
	LXC_TP_HIGHEST =				0x05, // 
};



enum lxc_cf_type {					// MGW_PT_CFG 0xB2, leaving out the old RS232 config values as the USB gateway exclusively talks USB :)
	LXC_CT_CONNEX = 				0x02, // set communications style, default seems to be LXC_CM_AUTO
	LXC_CT_SEND_OK_MRF =				0x04, // comes as a reply / ACK / NACK to each LXC_PT_TX sent
	LXC_CT_TIMEACCOUNT =				0x0A, // reads the current timeaccount in %. timeaccount is the time used sending / receiving
							      // packets as this is limited in the radio band 868MHz used by xcomfort to 1%
							      // 1% = 36 seconds each 1 hour, 1 packet takes 50ms => do the math how many packets this is per hour :)
	LXC_CT_COUNTER_RX = 				0x0B, // statistics, # of packets received since ???
	LXC_CT_COUNTER_TX = 				0x0C, // statistics, # of packets sent since ???
	LXC_CT_SERIAL = 				0x0E, // get serial number
	LXC_CT_RELEASE = 				0x1B, // get several internal release numbers
	LXC_CT_BACK_TO_FACTORY = 			0x1F, // obvious
};



enum lxc_cf_mode {					// MGW_PT_CFG 0xB2, details for cf_types (answer will be LXC_PT_STATUS packet)
	LXC_CM_AUTO =					0x01, // default
	LXC_CM_STATUS =					0x00, // for LXC_CT_CONNEX returns 0x00
							      // for LXC_CT_TIMEACCOUNT, returns 0x00 & LXC_PT_STATUS packet has remaining TX-packets in % within 1h (max 1000 packets / hour)
							      // for LXC_CT_SEND_OK returns 0x00
	LXC_CM_CLEAR = 					0x0F, // for LXC_CT_SEND_OK_MRF
	LXC_CM_SET = 					0x01, // for LXC_CT_SEND_OK_MRF (default value)
// 	LXC_CM_STATUS = 				0x00, // for LXC_CT_SEND_OK_MRF, already defined (some values are used in multiple configs)
	LXC_CM_GET =					0x00, // for LXC_CT_COUNTER_RX & LXC_CT_COUNTER_TX: get counters // for LXC_CT_RELEASE: get release numbers 
	LXC_CM_GET_REVISION = 				0x10, // for LXC_CT_RELEASE, get revision numbers
	LXC_CM_BTF_GW = 				0x0F, // reset back to factory gateway settings
	LXC_CM_BTF_MRF = 				0xF0, // reset back to factory radio settings
	LXC_CM_BTF_ALL = 				0xFF, // reset back to factory BOTH settings
};


enum lxc_rx_msg_type {					
							// radio data received by the USB gateways from any sensor device
							// identical with lxc_tx_event data
	LXC_RMT_UP = 					0x50, // short press on UP button (from: CTAA-01/xx, CTAA-02/xx, CHSZ-00/xx, CHSZ-01/xx, CHSZ-02/xx, CRMA-00/xx (Channel 0 and Channel 1))
	LXC_RMT_DOWN = 					0x51, // short press on DOWN button
	LXC_RMT_UP_PRESSED = 				0x54, // long press on UP button
	LXC_RMT_UP_RELEASED = 				0x55, // stopped pressing UP button
	LXC_RMT_DOWN_PRESSED = 				0x56, // long press on DOWN button
	LXC_RMT_DOWN_RELEASED = 			0x57, // stopped pressing DOWN button
							// BASIC MODE configuration
	LXC_RMT_STATUS =				0x70, // status data in rx_data, this is NOT routed so the USB gateway needs to be "close enough" to rx it!
	LXC_RMT_BASIC_MODE =				0x80, // ACK for assign or remove of a device
};


enum lxc_rx_data_type {					// values of LXC_RX_DATA_TYPE, various formats for numerical / time / date data..., data is in rx_data
	LXC_RDT_NO_DATA = 				0x00,
	LXC_RDT_PERCENT = 				0x01, // 1 byte
	LXC_RDT_SSHORT = 				0x03, // 2 bytes
	LXC_RDT_TIME =					0x1E, // 4 bytes, byte1=h / byte2=m / byte3=s / byte4=always 0 (data is binary, so 23h = 0x17)
};


enum lxc_rx_rssi {					// Received Signal Strength Indicator
	LXC_RSSI_GOOD =					67,  // 0 .. 67 = Good, 
	LXC_RSSI_NORMAL = 				75,  // 67 .. 75 = Normal, 
	LXC_RSSI_WEAK = 				95,  // 75 .. 90 = Weak
	LXC_RSSI_VERYWEAK = 				120, // 90 .. 120 = Very Weak	
};


enum lxc_rx_battery {					// Battery Status
	LXC_RB_NA =					0x00,  // no battery status provided
	LXC_RB_0 =					0x00,  // empty
	LXC_RB_25 =					0x00,  // low
	LXC_RB_50 =					0x00,  // weak
	LXC_RB_75 =					0x00,  // OK
	LXC_RB_100 =					0x00,  // new
	LXC_RB_PWR =					0x00,  // is on mains
};




union lxc_packet {					// the generic structure of a USB gateway communications packet, union of the 4 elements of enum lxc_pkt_type
	struct {					// pls the RAW DATA, using struct "raw" here to be consistent; raw is only used for hexdumping a packet
	uint8_t rawdata[LXC_MAX_PACKET_SIZE];
	} raw;

	struct {					// LXC_PT_TX packet 0xB1, this is the basic "command packet", it simulates a sensors (switch / dimmer / etc) output packet
	uint8_t len; 					// normally 6 (or 7 or 9, if 2 / 4 byte event_data is used)
	uint8_t type; 					// enum lxc_pkt_type, always LXC_PT_TX
	uint8_t tx_datapoint;				// Data point (1-99) of destination of radiogram
	uint8_t tx_event; 				// enum lxc_tx_event
	uint8_t tx_event_data;				// Can be 1 or 2 or 4 bytes! for the cmds I use here it is OK to have just 1 byte defined
	uint8_t tx_priority; 				// enum lxc_tx_priority
	} tx;

	struct {					// LXC_PT_CFG packet 0xB2, this packet type is for configuration / management purposes
	uint8_t len; 					// always 4
	uint8_t type; 					// enum lxc_pkt_type, always LXC_PT_CFG
	uint8_t cf_type;				// enum lxc_cf_type
	uint8_t cf_mode;				//
	} cfg;

	struct {					// LXC_PT_RX packet 0xC1, this is sent asynchronously by the USB gateway when the gateway reads radio data
	uint8_t len; 					// always 12
	uint8_t type; 					// enum lxc_pkt_type, always LXC_PT_RX
	uint8_t rx_datapoint;				// Data point (1-99) of origin of radiogram
	uint8_t rx_msg_type; 				// 
	uint8_t rx_data_type;				// the type of rx_data
	uint8_t rx_info_short;				// no enum,	data from ... means
							//		switching actor ... 0x00 = OFF, 0x01 = ON
							//		dimming actor ... 0..100 dim level in % 
							// 		jalousie actor ... 0x00 = STOP, 0x01 = OPEN, 0x02 = CLOSE 
							//			OPEN & CLOSE only sent during movement!
							//			calculate position of jalousie by counting time of OPEN or CLOSE packets
							//		basic mode (rx_msg_type = LXC_RMT_BASIC_MODE) ... 0x11 RF sensor was removed
							//		basic mode (rx_msg_type = LXC_RMT_BASIC_MODE) ... 0x12 RF sensor was assigned OK
							//		basic mode (rx_msg_type = LXC_RMT_BASIC_MODE) ... 0x13 RF sensor NOT assigned (list full)
							//		basic mode (rx_msg_type = LXC_RMT_BASIC_MODE) ... 0x14 RF sensor about to be removed
							//			you need to hold the sensor for at least 5 secs then it is removed, you get a rx packet:
							//			basic mode (rx_msg_type = LXC_RMT_BASIC_MODE) ... 0x11 RF sensor was removed
	uint32_t rx_data;				// Complete data value, format according to DATA_TYPE, High byte first
	uint8_t rx_rssi;				// enum lxc_rx_rssi,  Received Signal Strength Indication: < 50 very good; > 90 very bad
	uint8_t rx_battery; 				// enum lxc_rx_battery, Current Battery Status
	} rx;

	struct {					// LXC_PT_STA packet 0xC3, this is the immediate answer to any LXC_PT_CFG packet 0xB2
	uint8_t len; 					// the USB gateway will always set this to 8
	uint8_t type; 					// enum lxc_pkt_type, always LXC_PT_ACK = 0xC3
	uint8_t st_type;				// this repeats the LXC_PT_CFG packet cf_type
	uint8_t st_status;				// this repeats the LXC_PT_CFG packet cf_mode
	uint32_t st_data; 				// sometimes filled, else 0x00
	} sta;
};

/* NOTE ON INTEGRATING THE USB GATEWAY:

	To integrate the USBGW into your environment you have to discover it using the MRF software.
	The USBGW can remember up to 100 datapoints (00-99). You have to configure it like you
	would configure a pushbutton/sensor or an actor in the MRF SW. Otherwise it will NOT
	receive any data. I have tried it. You cant use it as a "RF packet sniffer" :(

	This means that you have to load the password into the USBGW, then load the datapoints
	and only then then you will receive LXC_PT_RX packets from these devices. Unfortunately
	you cant just tell the USBGW the password of your MRF system and then listen to whats
	going on in your network. You have to program the up to 100 connections into the USBGW
	(and I have no idea how I can do that with the limited graphical options of the MRF SW).

	My current system has 30 sensors and 60 actors. This will be a whole mess if I do the 
	programming by drawing "connection lines" in the MRF SW. If anyone has a better idea please 
	feel free to tell me.
*/

// init USB functions
libusb_device_handle 	*lxc_open();
void 			lxc_close(libusb_device_handle *dev);
void 			lxc_init(void);
void 			lxc_exit(void);
// compound functions
void 			lxc_get_stats(struct libusb_device_handle *dev);
int			lxc_assign_sensor(struct libusb_device_handle *dev, int dp);
int			lxc_poll_rx(struct libusb_device_handle *dev);
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

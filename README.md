# xcomfort
## Moeller EATON xComfort USB Gateway (USBGW) interface library

This work is heavily based on mifis xcomfort libraries (https://github.com/mifi/libxcomfort). He did extensive work for dimming. 

I took his code as my base and focused on some very core functionalities. I'm using the new USB dongle-style gateway (Eaton / Möller xComfort USB Gateway (communications interface) CKOZ-00/14) which seems to be a bit more sensitive to some USB settings (especially the buffer size). 

Thus Mifis code in the end didn't work for me, but provided a sound basis for how to open the usb device. This code is included nearly "as is" and I just added debugging facilities to it. 

## Library Contents

The programmatic interface to the USBGW was reworked completely. The xcomfort.c "library" now contains 

A) the code to open and close the USB device

B) the code to send and receive raw USB gateway radio packets (lxc_out and lxc_in)

C) the data structures for all 4 packet types supported by the USBGW, I defined constants for the most common commands and settings

D) sample code to retrieve some basic info from the USBGW (like serial number, revision, packets sent / received)

E) sample code to "teach" a sensor to control a given datapoint (actor), this demonstrates the programmatic use of "basic mode"; I did this to test the core functionality with just a sensor (pushbutton) and the USBGW

F) sample code to listen to the USBGW and other sensors and actors and log the packet traffic

The code right now is more or less work in progress and can serve as a starting point for other projects. It is in no form complete nor will it be completed to any given definition in the future. Use it as it is.

## NOTE ON INTEGRATING THE USB GATEWAY:

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


rgds,

cdedc

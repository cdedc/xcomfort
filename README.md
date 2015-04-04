# xcomfort
## Moeller EATON xComfort USB Gateway (USBGW) interface library

This work is heavily based on mifis xcomfort libraries (https://github.com/mifi/libxcomfort). He did extensive work for dimming. 

I took his code as my base and focused on some very core functionalities. I'm using the new USB dongle-style gateway (Eaton / Möller xComfort USB Gateway (communications interface) CKOZ-00/14) which seems to be a bit more sensitive to some USB settings (especially the buffer size). 

Thus Mifis code in the end didn't work for me, but provided a sound basis for how to open the usb device. This code is included nearly "as is" and I just added debugging facilities to it. 

The programmatic interface to the USBGW was reworked completely. The xcomfort.c "library" now contains 

1 the code toopen and close the USB device

B) the code tosend and receive raw USB gateway radio packets (lxc_out and lxc_in)

C) the data structures for all 4 packet types supported by the USBGW, I defined constants for the most common commands and settings

D) sample code to retrieve some basic info from the USBGW (like serial number, revision, packets sent / received)

E) sample code to "teach" a sensor to control a given datapoint (actor), this demonstrates the programmatic use of "basic mode"; I did this to test the core functionality with just a sensor (pushbutton) and the USBGW

F) sample code to listen to the USBGW and other sensors and actors and log the packet traffic

The code right now is more or less work in progress and can serve as a starting point for other projects. It is in no form complete nor will it be completed to any given definition in the future. Use it as it is.

rgds,

cdedc

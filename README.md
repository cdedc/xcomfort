# xcomfort
Moeller EATON xComfort USB Gateway interface library

This work is heavily based on mifis xcomfort libraries (https://github.com/mifi/libxcomfort). He did some work for dimming. 

I took his base and focused on basic functionality. I'm using the new USB dongle-style gateway (Eaton / Möller xComfort USB Gateway (communications interface) CKOZ-00/1) which seems to be a bit more sensitive to some settings (especially buffer size). 

Thus Mifis code in the end didn't work for me, but provided a sound basis for how to open the usb device. This code is included nearly "as is" and I just added debugging facilities to it. 

The programmatic interface to the USBGW was reworked completely. The xcomfort.c "library" now contains the code to 

A) open and close the USB device

B) send and receive raw USB gateway radio packets

C) the data structures for all 4 packet types supported by the USBGW

D) sample code to retrieve some basic info from the USBGW

E) sample code to "teach" a sensor to control a given datapoint (actor)

F) sample code to listen to the USBGW and other sensors and actors and log the packet traffic

The code right nowis more or less work in progress and can serve as a starting point for other projects. It is in no form complete.

rgds,

cdedc

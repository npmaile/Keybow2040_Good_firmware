# Keybow2040_Good_firmware

This is firmware I thought the device desparately needed after attempting to make a useable numpad/arrow macropad with the python implementation that comes with the device.

## Building
1. follow the instructions from the pico sdk documentation to get the toolchain running
1. clone this in a sibling folder to the pico sdk
1. `cd Keybow2040_Good_firmware && mkdir build && cd build`
1. `cmake ..`
1. make
## installing
1. put your pi pico into firmware programming mode by pressing the button and plugging it in
1. drag the uf2 file into the mass storage device the pico is pretending to be

## Philosophy
I think the default firmware with the python interpreter is nice for learning, but runs too slowly to make a serious companion device to sit on my desk. I attempted to make the same numpad functionality with the python apis, and it worked but would sometimes double-print letters. I have no idea why this was happening, but it happened less when i removed the lighting effects. This was proof enough to me that the chip is underpowered for a python based stack, so I've re-written it using the pico-sdk. 

## Attributions
LED driver firmware was lifted from Adafruit's libraries
The usb code here was lifted from the tinyusb examples and tweaked until I got it working

## AI
No AI produced code is in this repository, however AI assisted in telling me why I was stupid during the development as well as the writing of the github action

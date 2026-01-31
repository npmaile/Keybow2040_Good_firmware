#Keybow2040_Good_firmware

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

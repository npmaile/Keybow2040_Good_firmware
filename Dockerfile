FROM ubuntu:jammy AS builder

WORKDIR /root
RUN mkdir /root/Keybow2040_Good_firmware
RUN apt update
RUN apt install -y cmake python3 build-essential gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib git
RUN git clone https://github.com/raspberrypi/pico-sdk --recursive
COPY . /root/Keybow2040_Good_firmware
RUN PICO_SDK_PATH=/root/pico-sdk cmake ./Keybow2040_Good_firmware/
RUN make


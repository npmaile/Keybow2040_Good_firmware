cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(2040fw
	test.c
	usb_descriptors.c
	usb_kb.c
	led.c
)

target_sources(2040fw PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/test.c
	${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
	${CMAKE_CURRENT_LIST_DIR}/usb_kb.c
	${CMAKE_CURRENT_LIST_DIR}/led.c
)

target_include_directories(2040fw PUBLIC 
	${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(2040fw)

target_link_libraries(2040fw PUBLIC tinyusb_device tinyusb_board pico_multicore pico_rand pico_stdlib hardware_i2c pico_unique_id)

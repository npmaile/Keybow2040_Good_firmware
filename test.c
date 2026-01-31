#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/rand.h"

#include "hardware/gpio.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "keyinput.h"

#include "led.h"

static uint8_t keysAndGPIO[16] = {
	18,14,10,6,
	19,15,11,7,
	20,16,12,8,
	21,17,13,9
};

uint8_t keymap1[16] = {
	0,HID_KEY_7,HID_KEY_8,HID_KEY_9,
	0,HID_KEY_4,HID_KEY_5,HID_KEY_6,
	0,HID_KEY_1,HID_KEY_2,HID_KEY_3,
	0,HID_KEY_BACKSPACE,HID_KEY_0,HID_KEY_ENTER
};

uint8_t mode1Lights[16][3] = {
	{0,0,255},{0,255,0},{0,255,0},{0,255,0},
	{0,0,255},{0,255,0},{0,255,0},{0,255,0},
	{0,0,255},{0,255,0},{0,255,0},{0,255,0},
	{0,0,255},{255,0,0},{0,255,0},{255,255,0}
};


uint8_t keymap2[16] = {
	0,0,0,0,
	0,0,0,0,
	0,0,HID_KEY_ARROW_UP,0,
	0,HID_KEY_ARROW_LEFT,HID_KEY_ARROW_DOWN,HID_KEY_ARROW_RIGHT
};

uint8_t mode2Lights[16][3] = {
	{0,0,255},{0,0,0},{0,0,0},{0,0,0},
	{0,0,255},{0,0,0},{0,0,0},{0,0,0},
	{0,0,255},{0,0,0},{255,255,255},{0,0,0},
	{0,0,255},{255,255,255},{255,255,255},{255,255,255}
};

uint8_t active_keymap[16];
uint8_t active_lights[16][3];

static bool wasPressed[16] = {
	false,false,false,false,
	false,false,false,false,
	false,false,false,false,
	false,false,false,false
};

int main() {
	stdio_init_all();
	led_driver_init();
	init_keyboard_gpio();
	tud_init(BOARD_TUD_RHPORT);

	for (uint8_t i = 0; i < 144; i++){
		sleep_ms(10);
		setPixel(i,0,0);
	}
	memcpy(active_keymap,keymap1,16);

	push_bitmap(mode1Lights);
	while (true) {
		tud_task();
		hid_task();
		for (uint8_t i = 0; i < 16; i++){
			bool isPressed = !gpio_get(keysAndGPIO[i]);
			
			if (isPressed){
			switch (i){
			case 0:
				memcpy(active_keymap,keymap1,16);
				memcpy(active_lights,mode1Lights,16 * 3);
				push_bitmap(mode1Lights);
				break;
			case 4:
				memcpy(active_keymap,keymap2,16);
				memcpy(active_lights,mode2Lights,16 * 3);
				push_bitmap(mode2Lights);
				break;
			case 8:

				break;
			case 12:

				break;
			}
				Add_key_input(i);
				setKeyRGB(i,255,255,255);
				wasPressed[i] = true;
			}else if (!isPressed && wasPressed[i]) {
				uint8_t *colors = active_lights[i];
				setKeyRGB(i,colors[0],colors[1],colors[2]);
				wasPressed[i] = false;
			}
		}
	}
}

auto_init_mutex(keyBufMtex);

uint8_t KeyBuf[6] = {};
uint8_t bufPtr = 0;

void Add_key_input(int kb_index){
	mutex_enter_blocking(&keyBufMtex);
	if (bufPtr >=6) {
		mutex_exit(&keyBufMtex);
		return;
	}
	uint8_t keyCode = (active_keymap)[kb_index];
	if (keyCode != 0){
		KeyBuf[bufPtr++] = keyCode;
	}
	mutex_exit(&keyBufMtex);
}

bool Get_Key_Inputs(char buf[6]){
	mutex_enter_blocking(&keyBufMtex);
	if (bufPtr == 0 && KeyBuf[0] == 0){
		mutex_exit(&keyBufMtex);
		return false;
	}
	memcpy(buf,KeyBuf,6);
	memset(KeyBuf, 0, 6);
	bufPtr = 0;
	mutex_exit(&keyBufMtex);
	return true;
}

void init_keyboard_gpio(){
	for (uint8_t i = 0; i < 16; i++){
		uint8_t target = keysAndGPIO[i];
		gpio_init(target);
		gpio_set_input_enabled(target, true);
		gpio_pull_up(target);
	}
}

	


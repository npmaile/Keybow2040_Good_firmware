#include <stdio.h>
#include "pico/double.h"
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "led.h"

void mouse_jiggler_task();
void keyboard_task();
int8_t Squash32bit(int32_t in);
void setup_mouse_jiggler();
void setup_mouse_func();
void mouse_task();
void (*mouseShapeFunc)(int32_t in[2], double t);
void circleFunc(int32_t in[2],double t);
void butterflyFunc(int32_t in[2],double t);
void heartFunc(int32_t in[2],double t);

#define E_CONST 2.71828182845904523536
#define PI_CONST 3.1415926
#define KEY_DEBOUNCE_MS 100


static uint8_t keysAndGPIO[16] = {
	18,14,10,6,
	19,15,11,7,
	20,16,12,8,
	21,17,13,9
};

absolute_time_t lastPressed[16] = {
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};

bool is_pressed(uint8_t keyIndex, absolute_time_t timeNow){
	// check to see if the last time it's pressed is less than KEY_DEBOUNCE_MS in the past
	if (absolute_time_diff_us(lastPressed[keyIndex],timeNow) < KEY_DEBOUNCE_MS * 1000){
		return true;
	}
	if (gpio_get(keysAndGPIO[keyIndex])){
		return false;
	}
	lastPressed[keyIndex] = timeNow;
	return true;
}

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

void (*ktask)(void);

void setup_keyboard_task1(){
	memcpy(active_keymap,keymap1,16);
	memcpy(active_lights,mode1Lights,16 * 3);
	push_bitmap(mode1Lights);
	ktask = &keyboard_task;
}

void setup_keyboard_task2(){
	memcpy(active_keymap,keymap2,16);
	memcpy(active_lights,mode2Lights,16 * 3);
	push_bitmap(mode2Lights);
	ktask = &keyboard_task;
}

void keyboard_task(){
	absolute_time_t timeNow = get_absolute_time();
	for (uint8_t i = 0; i < 16; i++){
		bool isPressed = is_pressed(i, timeNow);
			
		if (isPressed){
			switch (i){
			case 0:
				setup_keyboard_task1();
				return;
			case 4:
				setup_keyboard_task2();
				return;
			case 8:
				setup_mouse_jiggler();
				return;
			case 12:

				break;
			}
			Add_Key_Input(active_keymap[i]);
			setKeyRGB(i,255,255,255);
			wasPressed[i] = true;
		}else if (!isPressed && wasPressed[i]) {
			uint8_t *colors = active_lights[i];
			setKeyRGB(i,colors[0],colors[1],colors[2]);
			wasPressed[i] = false;
		}
	}

}

void setup_mouse_func(){
	push_bitmap(mode2Lights);
	ktask = &mouse_task;
}

uint64_t t_0 = 0;
double current_dir_radians = 0;
double speed = 0;
void mouse_task(){
	absolute_time_t now = get_absolute_time();
	uint64_t t_1 =  to_us_since_boot(now);
	if ((t_1 - t_0) < 10000){
		return;
	}
	t_0 = t_1;
	// key handling
	if (is_pressed(0,now)) {
		setup_keyboard_task1();
		return;
	}
	if (is_pressed(4,now)){
		setup_keyboard_task2();
		return;
	}
	if (is_pressed(8,now)){
		setup_mouse_jiggler();
		return;
	}
	if (is_pressed(10,now)&& speed < 128){
		speed += .1 ;
	}
	if (is_pressed(13,now)){
		current_dir_radians += .01*PI_CONST;
	}
	if (is_pressed(14,now) && speed > -128){
		speed -= .1;
	}
	if (is_pressed(15,now)){
		current_dir_radians -= .01*PI_CONST;
	}
	// mouse input
	int8_t delta_x = floor(sin(current_dir_radians) * speed *.2);
	int8_t delta_y = floor(cos(current_dir_radians) * speed)*.2;
	Add_Mouse_Input(Squash32bit(delta_x),Squash32bit(delta_y));
	// lights
	uint64_t t_2 =  to_us_since_boot(get_absolute_time())/ 100000;
	uint8_t arrowKeys[4] = {15,14,13,10};
	for (uint8_t i = 0; i < 4; i ++){
		uint8_t r = floor(255 * sin(t_2+(i * .01)));
		uint8_t g = floor(255 * sin(t_2+(i * .80)));
		uint8_t b = floor(255 * sin(t_2+(i * .50)));
		setKeyRGB(arrowKeys[i],r,g,b);
	}
}

void setup_mouse_jiggler(){
	ktask = &mouse_jiggler_task;
	mouseShapeFunc = &circleFunc;
}

double SCALING_FACTOR = 300;
double TIME_SLOW_FACTOR = 100000;
void mouse_jiggler_task(){
	absolute_time_t now = get_absolute_time();
	uint64_t t_1 =  to_us_since_boot(now);
	if ((t_1 - t_0) < 10000){
		return;
	}
	t_0 = t_1;
	static int32_t mouse_x = 0;
	static int32_t mouse_y = 0;

	for (uint8_t i = 0; i < 16; i++){
		bool isPressed = is_pressed(i,now);
		if (!isPressed){
			continue;
		}
		switch(i){
		case 0:
			setup_keyboard_task1();
			return;
		case 1:
			SCALING_FACTOR --;
			break;
		case 2: 
			SCALING_FACTOR = 300;
			break;
		case 3: 
			SCALING_FACTOR ++;
			break;
		case 4:
			setup_keyboard_task2();
			return;
		case 5:
			TIME_SLOW_FACTOR = TIME_SLOW_FACTOR + 100;
			return;
		case 6:
			TIME_SLOW_FACTOR = 100000;
			return;
		case 7:
			TIME_SLOW_FACTOR = TIME_SLOW_FACTOR - 100;
			return;
		case 8:
			mouseShapeFunc = &circleFunc;
			break;
		case 9:
			mouseShapeFunc = &butterflyFunc;
			break;
		case 10:
			setup_mouse_func();
			return;
		case 11:
			mouseShapeFunc = &heartFunc;
			break;
		case 12:
			break;
		case 13:
			setup_mouse_func();
			break;
		case 14:
			setup_mouse_func();
			break;
		case 15:
			setup_mouse_func();
			break;
		default:
			// also do nothing
			break;
		}
	}
	double t = t_1 / TIME_SLOW_FACTOR;
	for (uint8_t i = 0; i < 16; i++){
		uint8_t r = floor(255 * sin(t+(i * .01)));
		uint8_t g = floor(255 * sin(t+(i * .80)));
		uint8_t b = floor(255 * sin(t+(i * .50)));

		setKeyRGB(i,r,b,g);
	}

	int32_t xy[2] = {0};
	mouseShapeFunc(xy, t);
	int32_t delta_x = Squash32bit(mouse_x - xy[0]);
	int32_t delta_y = Squash32bit(mouse_y - xy[1]);
	
	Add_Mouse_Input(delta_x,delta_y);
	mouse_x -= delta_x;
	mouse_y -= delta_y;
}

void butterflyFunc(int32_t in[2], double t){
	in[0] = floor((sin(t) * (pow(E_CONST,cos(t)) - 2 * cos(4 * t) - pow(sin(t/12),5))) * SCALING_FACTOR * .1);
	in[1] = floor((cos(t) * (pow(E_CONST,cos(t)) - 2 * cos(4 * t) - pow(sin(t/12),5))) * SCALING_FACTOR * .1);
}
void heartFunc(int32_t in[2], double t){
	in[0] = floor((16 * pow(sin(t), 3.0)) * SCALING_FACTOR * .1);
	in[1] = floor((13 * cos(t) - 5 * cos(2 *t) - 2 * cos(3* t) - cos(4 * t)) * SCALING_FACTOR * .1);
}

void circleFunc(int32_t in[2],double t){
	in[0] = floor(sin(t) * SCALING_FACTOR);
	in[1] = floor(cos(t) * SCALING_FACTOR);
}

int8_t Squash32bit(int32_t in){
	return (in > 127)?127:(in<-127)?-127:(int8_t)in;
}

int main() {
	stdio_init_all();
	led_driver_init();
	init_keyboard_gpio();
	tud_init(BOARD_TUD_RHPORT);

	setup_keyboard_task1();
	while (true) {
		tud_task();
		hid_task();
		ktask();
	}
}

void init_keyboard_gpio(){
	for (uint8_t i = 0; i < 16; i++){
		uint8_t target = keysAndGPIO[i];
		gpio_init(target);
		gpio_set_input_enabled(target, true);
		gpio_pull_up(target);
	}
}


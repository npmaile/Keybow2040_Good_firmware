#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

struct LightCommand;

typedef struct LightCommand {
  uint8_t index;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} LightCMD;

void writeRegister8(uint8_t bank, uint8_t reg, uint8_t data);

void selectBank(uint8_t bank);

void clear();

void setPixel(uint8_t num, uint8_t pwm, uint8_t bank);

void led_driver_init();

void setKeyRGB(uint8_t key, uint8_t r, uint8_t g, uint8_t b);

void push_bitmap(uint8_t array_of_lights[][3]);

void sendKeyRGB(LightCMD);

#endif

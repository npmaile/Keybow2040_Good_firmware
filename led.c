#include "led.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/multicore.h"
#include "stdint.h"
#include <string.h>

#define LED_DRIVER_BUS_ADDRESS 0x74

#define ISSI_REG_CONFIG 0x00
#define ISSI_REG_CONFIG_PICTUREMODE 0x00
#define ISSI_REG_CONFIG_AUTOPLAYMODE 0x08
#define ISSI_REG_CONFIG_AUDIOPLAYMODE 0x18

#define ISSI_CONF_PICTUREMODE 0x00
#define ISSI_CONF_AUTOFRAMEMODE 0x04
#define ISSI_CONF_AUDIOMODE 0x08

#define ISSI_REG_PICTUREFRAME 0x01

#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_AUDIOSYNC 0x06

#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B
// list of all the pixels with their red, green, and blue
uint8_t lookup[16][3] = {
    {120, 88, 104}, // 0, 0
    {136, 40, 72},  // 1, 0
    {112, 80, 96},  // 2, 0
    {128, 32, 64},  // 3, 0
    {121, 89, 105}, // 0, 1
    {137, 41, 73},  // 1, 1
    {113, 81, 97},  // 2, 1
    {129, 33, 65},  // 3, 1
    {122, 90, 106}, // 0, 2
    {138, 25, 74},  // 1, 2
    {114, 82, 98},  // 2, 2
    {130, 17, 66},  // 3, 2
    {123, 91, 107}, // 0, 3
    {139, 26, 75},  // 1, 3
    {115, 83, 99},  // 2, 3
    {131, 18, 67},  // 3, 3
};

void writeRegister8(uint8_t bank, uint8_t reg, uint8_t data) {
  selectBank(bank);
  uint8_t cmd[2] = {reg, data};
  i2c_write_blocking(i2c_default, LED_DRIVER_BUS_ADDRESS, cmd, 2, false);
}

void selectBank(uint8_t bank) {
  uint8_t cmd[2] = {ISSI_COMMANDREGISTER, bank};
  i2c_write_blocking(i2c_default, LED_DRIVER_BUS_ADDRESS, cmd, 2, false);
}

void setPixel(uint8_t num, uint8_t pwm, uint8_t bank) {
  writeRegister8(0, 0x24 + num, pwm);
}

void setKeyRGB(uint8_t key, uint8_t r, uint8_t g, uint8_t b) {
  setPixel(lookup[key][0], r, 0);
  setPixel(lookup[key][1], g, 0);
  setPixel(lookup[key][2], b, 0);
}

void led_driver_init() {
  i2c_init(i2c_default, 400 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);

  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);

  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG,
                 ISSI_REG_CONFIG_PICTUREMODE);

  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, 0x00);

  // turn on all of the LEDs
  for (uint8_t f = 0; f < 8; f++) {
    for (uint8_t i = 0; i <= 0x11; i++)
      writeRegister8(f, i, 0xff); // each 8 LEDs on
  }

  for (uint8_t i = 0; i < 144; i++) {
    setPixel(i, 0, 0);
  }
  LightCMD toSet;
  uint32_t data;
  while (true) {
    data = multicore_fifo_pop_blocking();
    memcpy(&toSet, &data, sizeof(LightCMD));
    setKeyRGB(toSet.index, toSet.r, toSet.g, toSet.b);
  }
}

void push_bitmap(uint8_t array_of_lights[][3]) {
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t *ptr = array_of_lights[i];
    sendKeyRGB((LightCMD){i, ptr[0], ptr[1], ptr[2]});
  }
}

void sendKeyRGB(LightCMD cmd) {
  static uint32_t buf;
  memcpy(&buf, &cmd, sizeof(LightCMD));
  multicore_fifo_push_blocking(buf);
}

#ifndef USB_KB_H
#define USB_KB_H


#include <stdint.h>
#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);
static void send_hid_report(uint8_t report_id);
void hid_task(void);
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
bool Get_Key_Inputs(char buf[6]);
void Add_Key_Input(int kb_index);
void init_keyboard_gpio();
void Add_Mouse_Input(uint8_t x, uint8_t y);
void Get_Mouse_Input(uint8_t buf[2]);
#endif

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/rand.h"

#include "hardware/gpio.h"

#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "usb_kb.h"

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{

}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
	uint8_t keyCodes[6] = {0};
	if (Get_Key_Inputs(keyCodes)){
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keyCodes);
	}else{
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
	}
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t delta[2] = {0};
	Get_Mouse_Input(delta);
      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta[0], delta[1], 0, 0);
    }
	default: break;
  }
}

// we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  const uint32_t interval_ms = 2;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_KEYBOARD);
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id);
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
}


// nate wrote these functions in their entirety to make stuff easier

auto_init_mutex(keyBufMtex);

uint8_t KeyBuf[6] = {};
uint8_t bufPtr = 0;

void Add_Key_Input(int keyInput){
	mutex_enter_blocking(&keyBufMtex);
	if (bufPtr >=6) {
		mutex_exit(&keyBufMtex);
		return;
	}
	if (keyInput != 0){
		KeyBuf[bufPtr++] = keyInput;
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

auto_init_mutex(mouseBufMtex);
uint8_t mouse_inputs[2] = {0,0};

void Add_Mouse_Input(uint8_t x, uint8_t y){
	mutex_enter_blocking(&mouseBufMtex);

	mouse_inputs[0] = x;
	mouse_inputs[1] = y;

	mutex_exit(&mouseBufMtex);
}

void Get_Mouse_Input(uint8_t buf[2]){
	mutex_enter_blocking(&mouseBufMtex);

	buf[0] = mouse_inputs[0];
	mouse_inputs[0] = 0;
	buf[1] = mouse_inputs[1];
	mouse_inputs[1] = 0;

	mutex_exit(&mouseBufMtex);
}


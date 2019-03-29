#include "wiimote.h"

unsigned short wiimote_vendor_id = 0x057e;
unsigned short wiimote_product_id = 0x0306;

wiimote* wiimote_new(const wchar_t* serial){
  wiimote* ret = malloc(sizeof(wiimote));
  ret->buttons.buttons = 0;
  ret->rumble = false;
  ret->handle = hid_open(wiimote_vendor_id, wiimote_product_id, serial);

  if(ret->handle == NULL){
    // couldn't open HID device
    free(ret);
    return NULL;
  }

  return ret;
}

int wiimote_read(wiimote* w, unsigned char* buffer, int len){
  return hid_read(w->handle, buffer, len * sizeof(unsigned char));
}

int wiimote_write(wiimote* w, unsigned char* buffer, int len){
  buffer[1] |= w->rumble;
  return hid_write(w->handle, buffer, len * sizeof(unsigned char));
}

int wiimote_set_leds(wiimote* w, unsigned char led_state){
  unsigned char buffer[2];
  buffer[0] = 0x11;
  buffer[1] = led_state;
  return wiimote_write(w, buffer, 2);
}

void wiimote_parse_core_buttons(wiimote_core_buttons* buttons, unsigned char* buffer){
  buttons->buttons_first = buffer[1];
  buttons->buttons_second = buffer[2];
}

int wiimote_set_reporting_mode(wiimote* w, unsigned char reporting_mode, bool continuous){
  unsigned char buffer[3];
  buffer[0] = 0x12;
  buffer[1] = (continuous & 0x01) << 2;
  buffer[2] = reporting_mode;
  w->reporting_mode = reporting_mode;
  return wiimote_write(w, buffer, 3);
}

int wiimote_get_status_report(wiimote* w){
  unsigned char buffer[2];
  buffer[0] = 0x15;
  buffer[1] = 0x00;
  return wiimote_write(w, buffer, 2);
}

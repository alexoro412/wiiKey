#include "wiimote.h"

unsigned short wiimote_vendor_id = 0x057e;
unsigned short wiimote_product_id = 0x0306;

wiimote* wiimote_new(const wchar_t* serial){
  wiimote* ret = malloc(sizeof(wiimote));
  ret->buttons.buttons = 0;
  ret->rumble = false;
  ret->handle = hid_open(wiimote_vendor_id, wiimote_product_id, serial);
  ret->reporting_mode = 0x30;

  ret->reading_memory = false;
  ret->memory_address = 0;
  ret->memory = calloc(0x1700, sizeof(unsigned char));

  if(ret->handle == NULL){
    // couldn't open HID device
    free(ret);
    return NULL;
  }

  return ret;
}

void wiimote_free(wiimote* w){
  free(w->memory);
  free(w);
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

int wiimote_request_status_report(wiimote* w){
  unsigned char buffer[2];
  buffer[0] = 0x15;
  buffer[1] = 0x00;
  return wiimote_write(w, buffer, 2);
}

// currently limited to eeprom
int wiimote_request_memory(wiimote* w, unsigned short address, unsigned short size, bool read_from_register){

  if(w->reading_memory){
    // TODO maybe queue memory reads?
    return -1;
  }

  w->memory_address = address;
  w->memory_end_address = address + size - 1;
  printf("%d to %d\n", w->memory_address, w->memory_end_address);
  w->reading_memory = true;

  unsigned char buffer[7];
  buffer[0] = 0x17;
  buffer[1] = read_from_register << 2;
  // the address is stored as three bytes in big endian order
  buffer[2] = 0; //(address >> 16) & 255;
  buffer[3] = (address >> 8) & 255;
  buffer[4] = (address) & 255;
  // the size is also in big endian order
  buffer[5] = (size >> 8) & 255;
  buffer[6] = (size) & 255;
  return wiimote_write(w, buffer, 7);
}

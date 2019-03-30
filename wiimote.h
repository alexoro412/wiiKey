#ifndef __WIIMOTE_H__
#define __WIIMOTE_H__

#include <stdlib.h>
#include <hidapi.h>
#include <stdbool.h>
#include <string.h>

// TODO is this better than using a macro?
extern unsigned short wiimote_vendor_id;
extern unsigned short wiimote_product_id;

typedef struct {
  union {
    // The core buttons data as a single short
    unsigned short buttons;

    // The two bytes that represent the core buttons data
    struct {
      unsigned char buttons_first;
      unsigned char buttons_second;
    };

    // The individual buttons
    struct {
      bool left : 1;
      bool right : 1;
      bool down : 1;
      bool up : 1;
      bool plus : 1;
      int unused : 3;
      bool two : 1;
      bool one : 1;
      bool B : 1;
      bool A : 1;
      bool minus : 1;
      int _unused : 2;
      bool home : 1;
    };
  };
} wiimote_core_buttons;

// TODO what else should be stored in this struct?
// potential candidates:
// - led state
// - serial number / name
typedef struct {
  wiimote_core_buttons buttons;
  bool rumble;
  hid_device* handle;
  unsigned char reporting_mode;

  bool reading_memory;
  unsigned short memory_address;
  unsigned short memory_end_address;
  unsigned char* memory;
} wiimote;

// Corresponds to the 0x20 report
typedef struct {
  union{
    struct{
      bool low_battery : 1;
      bool extension_connected : 1;
      bool speaker_enabled : 1;
      bool ir_camera_enabled : 1;
      bool led1 : 1;
      bool led2 : 1;
      bool led3 : 1;
      bool led4 : 1;
    };
    unsigned char lf_byte; 
  };
  unsigned char battery_level;
} wiimote_status_report;

wiimote* wiimote_new(const wchar_t* serial);

void wiimote_free(wiimote* w);

int wiimote_read(wiimote* w, unsigned char* buffer, int len);

int wiimote_write(wiimote* w, unsigned char* buffer, int len);

enum leds {
  LED_OFF = 0x00,
  LED_1 = 0x10,
  LED_2 = 0x20,
  LED_3 = 0x40,
  LED_4 = 0x80
};

int wiimote_set_leds(wiimote* w, unsigned char led_state);

void wiimote_parse_core_buttons(wiimote_core_buttons* buttons, unsigned char* buffer);

int wiimote_set_reporting_mode(wiimote* w, unsigned char reporting_mode, bool continuous);

int wiimote_request_status_report(wiimote* w);

int wiimote_request_memory(wiimote* w, unsigned short address, unsigned short size, bool read_from_register);

#endif

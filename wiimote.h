#ifndef __WIIMOTE_H__
#define __WIIMOTE_H__

#include <stdlib.h>
#include <hidapi.h>
#include <stdbool.h>
#include <string.h>

extern unsigned short wiimote_vendor_id;
extern unsigned short wiimote_product_id;
/*
unsigned short wiimote_vendor_id = 0x057e;
unsigned short wiimote_product_id = 0x0306;
*/
typedef enum {
  wiimote_left = 1 << 8,
  wiimote_right = 1 << 9,
  wiimote_down = 1 << 10,
  wiimote_up = 1 << 11,
  wiimote_plus = 1 << 12,
  wiimote_2 = 1,
  wiimote_1 = 1 << 1,
  wiimote_B = 1 << 1,
  wiimote_A = 2 
} wiimote_button;

typedef struct {
  union {
    unsigned short buttons;
    struct {
      unsigned char buttons_first;
      unsigned char buttons_second;
    };
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


typedef struct {
  wiimote_core_buttons buttons;
  bool rumble;
  hid_device* handle;
  unsigned char reporting_mode;
} wiimote;

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
} wiimote_status_report;

wiimote* wiimote_new(const wchar_t* serial);

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


int wiimote_get_status_report(wiimote* w);
/*
struct wiimote_info {
  wchar_t* serial_number;
  struct wiimote_info* next;
};

typedef struct wiimote_info wiimote_info;

wiimote_info* enumerate_remotes();

void wiimote_free_enumeration(wiimote_info* list);
*/

#endif

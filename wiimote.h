#ifndef __WIIMOTE_H__
#define __WIIMOTE_H__

#include <stdlib.h>
#include <hidapi.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// TODO is this better than using a macro?
extern uint16_t wiimote_vendor_id;
extern uint16_t wiimote_product_id;

enum wiimote_memory_type {
  MEMORY_SPEAKER = 0xa2,
  MEMORY_EXTENSION = 0xa4,
  MEMORY_WII_MOTION_PLUS = 0xa6,
  MEMORY_IR_CAMERA = 0xb0,
  MEMORY_EEPROM = 0x00
};

typedef struct {
  union {
    // The core buttons data as a single short
    uint16_t buttons;

    // The two bytes that represent the core buttons data
    struct {
      uint8_t buttons_first;
      uint8_t buttons_second;
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
  uint8_t reporting_mode;

  bool extension_connected;

  bool reading_memory;
  enum wiimote_memory_type memory_type;
  uint16_t memory_address;
  uint16_t memory_end_address;
  uint8_t* eeprom;
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
    uint8_t lf_byte; 
  };
  uint8_t battery_level;
} wiimote_status_report;

wiimote* wiimote_new(const wchar_t* serial);

void wiimote_free(wiimote* w);

int wiimote_read(wiimote* w, uint8_t* buffer, int len);

int wiimote_write(wiimote* w, uint8_t* buffer, int len);

enum leds {
  LED_OFF = 0x00,
  LED_1 = 0x10,
  LED_2 = 0x20,
  LED_3 = 0x40,
  LED_4 = 0x80
};


int wiimote_set_leds(wiimote* w, enum leds led_state);

void wiimote_parse_core_buttons(wiimote_core_buttons* buttons, uint8_t* buffer);

int wiimote_set_reporting_mode(wiimote* w, uint8_t reporting_mode, bool continuous);

int wiimote_request_status_report(wiimote* w);

// Reading and writing to memory on the wiimote

int wiimote_request_memory(wiimote* w, uint16_t address, uint16_t size, enum wiimote_memory_type memory_type);

void wiimote_write_memory(wiimote* w, uint16_t address, uint16_t size, enum wiimote_memory_type memory_type, uint8_t* bytes);

int wiimote_write_memory_raw(wiimote* w, uint16_t address, uint8_t size, enum wiimote_memory_type memory_type, uint8_t* bytes);

void wiimote_initialize_speaker(wiimote* w);

void wiimote_shutdown_speaker(wiimote* w);

void wiimote_speaker_data(wiimote* w, uint8_t* buffer, int size);

#endif

#include "wiimote.h"

uint16_t wiimote_vendor_id = 0x057e;
uint16_t wiimote_product_id = 0x0306;

wiimote* wiimote_new(const wchar_t* serial){
  wiimote* ret = malloc(sizeof(wiimote));
  ret->buttons.buttons = 0;
  ret->rumble = false;
  ret->handle = hid_open(wiimote_vendor_id, wiimote_product_id, serial);
  ret->reporting_mode = 0x30;

  ret->reading_memory = false;
  ret->memory_address = 0;
  ret->eeprom = calloc(0x1700, sizeof(uint8_t));

  if(ret->handle == NULL){
    // couldn't open HID device
    free(ret);
    return NULL;
  }

  return ret;
}

void wiimote_free(wiimote* w){
  free(w->eeprom);
  free(w);
}

int wiimote_read(wiimote* w, uint8_t* buffer, int len){
  return hid_read(w->handle, buffer, len * sizeof(uint8_t));
}

int wiimote_write(wiimote* w, uint8_t* buffer, int len){
  buffer[1] |= w->rumble;
  return hid_write(w->handle, buffer, len * sizeof(uint8_t));
}

int wiimote_set_leds(wiimote* w, enum leds led_state){
  uint8_t buffer[2];
  buffer[0] = 0x11;
  buffer[1] = led_state;
  return wiimote_write(w, buffer, 2);
}

void wiimote_parse_core_buttons(wiimote_core_buttons* buttons, uint8_t* buffer){
  buttons->buttons_first = buffer[1];
  buttons->buttons_second = buffer[2];
}

int wiimote_set_reporting_mode(wiimote* w, uint8_t reporting_mode, bool continuous){
  uint8_t buffer[3];
  buffer[0] = 0x12;
  buffer[1] = (continuous & 0x01) << 2;
  buffer[2] = reporting_mode;
  w->reporting_mode = reporting_mode;
  return wiimote_write(w, buffer, 3);
}

int wiimote_request_status_report(wiimote* w){
  uint8_t buffer[2];
  buffer[0] = 0x15;
  buffer[1] = 0x00;
  return wiimote_write(w, buffer, 2);
}

int wiimote_request_memory(wiimote* w, uint16_t address, uint16_t size, enum wiimote_memory_type memory_type){

  // only allow one memory read at a time
  // TODO queue memory reads in the future
  if(w->reading_memory) return -1;

  bool read_from_register = false;
  switch(memory_type){
    case MEMORY_EEPROM:
      read_from_register = false;
      if(address >= 0x1700) return -1;
      break;

    case MEMORY_EXTENSION:
      if(address >= 0x0100) return -1;
      if(w->extension_connected){
        read_from_register = true;
      } else {
        return -1;
      }
      break;

    case MEMORY_SPEAKER:
      read_from_register = true;
      if(address > 0x0a) return -1;
      break;

    case MEMORY_IR_CAMERA:
      read_from_register = true;
      if(address > 0x34) return -1;
      break;

    case MEMORY_WII_MOTION_PLUS:
      printf("Fatal: Wii Motion+ not yet supported\n");
      exit(1);
      break;
    default:
      printf("Unrecognized memory type");
      return -1;
      break;
  }

  w->reading_memory = true;
  w->memory_type = memory_type;

  w->memory_address = address;
  w->memory_end_address = address + size;

  uint8_t buffer[7];
  
  buffer[0] = 0x17;
  buffer[1] = read_from_register << 2;
  
  buffer[2] = memory_type;
  buffer[3] = (address >> 8) & 255;
  buffer[4] = address & 255;

  // TODO verify size is correct
  buffer[5] = (size >> 8) & 255;
  buffer[6] = size & 255;

  return wiimote_write(w, buffer, 7);

}

// TODO have this return something
void wiimote_write_memory(wiimote* w, uint16_t address, uint16_t size, enum wiimote_memory_type memory_type, uint8_t* bytes){
  int number_of_full_packets = size / 16;
  int last_packet_size = size % 16;

  for(int i = 0; i < number_of_full_packets; i++){
    wiimote_write_memory_raw(w, address + i * 16, 16, memory_type, &(bytes[i*16]));
  }

  if(last_packet_size > 0){
    wiimote_write_memory_raw(w, address + number_of_full_packets * 16, last_packet_size, memory_type, &(bytes[number_of_full_packets * 16]));
  }
}


int wiimote_write_memory_raw(wiimote* w, uint16_t address, uint8_t size, enum wiimote_memory_type memory_type, uint8_t* bytes){
  uint8_t buffer[22];
  
  for(int i = 0; i < 22; i++) buffer[i] = 0;
  
  bool write_to_register = false;

  switch(memory_type){
    case MEMORY_EEPROM:
      write_to_register = false;
      if(address >= 0x1700) return -1;
      break;

    case MEMORY_EXTENSION:
      if(address >= 0x0100) return -1;
      if(w->extension_connected){
        write_to_register = true;
      } else {
        return -1;
      }
      break;

    case MEMORY_SPEAKER:
      write_to_register = true;
      if(address > 0x0a) return -1;
      break;

    case MEMORY_IR_CAMERA:
      write_to_register = true;
      if(address > 0x34) return -1;
      break;

    case MEMORY_WII_MOTION_PLUS:
      printf("Fatal: Wii Motion+ not yet supported\n");
      exit(1);
      break;
    default:
      printf("Unrecognized memory type");
      return -1;
      break;
  }


  buffer[0] = 0x16;
  buffer[1] = write_to_register << 2;
  buffer[2] = memory_type;
  buffer[3] = (address >> 8) & 255;
  buffer[4] = address & 255;
  buffer[5] = size;
  for(uint8_t i = 0; i < size; i++){
    buffer[6+i] = bytes[i];
    if(memory_type == MEMORY_EEPROM) w->eeprom[address + i] = bytes[i];
  }
  return wiimote_write(w, buffer, 22);
}


// TODO figure out why this sometimes fails
void wiimote_initialize_speaker(wiimote* w){
  uint8_t buffer[7];

  // enable speaker
  buffer[0] = 0x14;
  buffer[1] = 0x04;
  assert(2 == wiimote_write(w, buffer, 2));

  // printf("%d\n", res);

  // mute speaker
  buffer[0] = 0x19;
  buffer[1] = 0x04;
  assert(2 == wiimote_write(w, buffer, 2));

  // printf("%d\n", res);

  buffer[0] = 0x01;
  wiimote_write_memory(w, 0x0009, 1, MEMORY_SPEAKER, buffer);

  buffer[0] = 0x08;
  wiimote_write_memory(w, 0x0001, 1, MEMORY_SPEAKER, buffer);

  // set wiimote to play 4 bit ADPCM
  buffer[0] = 0x00;
  buffer[1] = 0x00;
  buffer[2] = 0xd0;
  buffer[3] = 0x07;
  buffer[4] = 0x40;
  buffer[5] = 0x00;
  buffer[6] = 0x00;
  wiimote_write_memory(w, 0x0001, 7, MEMORY_SPEAKER, buffer);

  buffer[0] = 0x01;
  wiimote_write_memory(w, 0x0008, 1, MEMORY_SPEAKER, buffer);

  // unmute speaker
  buffer[0] = 0x19;
  buffer[1] = 0x00;
  assert(2 == wiimote_write(w, buffer, 2));

  // printf("%d\n", res);
}

void wiimote_shutdown_speaker(wiimote* w){
  uint8_t buffer[2];
  buffer[0] = 0x14;
  buffer[1] = 0x00;

  wiimote_write(w, buffer, 2);
}

void wiimote_speaker_data(wiimote* w, uint8_t* data, int size){
  uint8_t buffer[22];
  buffer[0] = 0x18;
  buffer[1] = size << 3; // see docs about this; why?
  for(int i = 0; i < size; i++){
    buffer[2+i] = data[i];
  }
  for(int i = size; i < 20; i++){
    buffer[2+i] = 0x00;
  }

  wiimote_write(w, buffer, 22);
}

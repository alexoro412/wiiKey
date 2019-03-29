#include "keyboard.h"


void keyboard_key_down(int key) {
  CGEventRef evt = CGEventCreateKeyboardEvent(NULL, (CGKeyCode) key, true);
  CGEventPost(kCGHIDEventTap, evt);
  CFRelease(evt); 
}

void keyboard_key_up(int key) {
  CGEventRef evt = CGEventCreateKeyboardEvent(NULL, (CGKeyCode) key, false);
  CGEventPost(kCGHIDEventTap, evt);
  CFRelease(evt);
}

void keyboard_key_press(int key){
  keyboard_key_down(key);
  keyboard_key_up(key);
}

void keyboard_key_set(int key, bool down){
  if(down){
    keyboard_key_down(key);
  } else {
    keyboard_key_up(key);
  }
}

int keyboard_key_code_map[] = {
  kVK_ANSI_A,
  kVK_ANSI_B,
  kVK_ANSI_C,
  kVK_ANSI_D,
  kVK_ANSI_E,
  kVK_ANSI_F,
  kVK_ANSI_G,
  kVK_ANSI_H,
  kVK_ANSI_I,
  kVK_ANSI_J,
  kVK_ANSI_K,
  kVK_ANSI_L,
  kVK_ANSI_M,
  kVK_ANSI_N,
  kVK_ANSI_O,
  kVK_ANSI_P,
  kVK_ANSI_Q,
  kVK_ANSI_R,
  kVK_ANSI_S,
  kVK_ANSI_T,
  kVK_ANSI_U,
  kVK_ANSI_V,
  kVK_ANSI_W,
  kVK_ANSI_X,
  kVK_ANSI_Y,
  kVK_ANSI_Z,
  kVK_ANSI_0,
  kVK_ANSI_1,
  kVK_ANSI_2,
  kVK_ANSI_3,
  kVK_ANSI_4,
  kVK_ANSI_5,
  kVK_ANSI_6,
  kVK_ANSI_7,
  kVK_ANSI_8,
  kVK_ANSI_9
};

int keyboard_key_code(char key){
  if('A' <= key && key <= 'Z')
    key += 'A' - 'a';

  return keyboard_key_code_map[key - 'a'];
}

/*keyboard keyboard_new(){
  return CGE
}*/

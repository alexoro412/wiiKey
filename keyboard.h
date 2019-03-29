#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

void keyboard_key_up(int key);

void keyboard_key_down(int key);

void keyboard_key_set(int key, bool down);

void keyboard_key_press(int key);

int keyboard_key_code(char key);


#endif // KEYBOARD_H

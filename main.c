#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <hidapi.h>
#include "keyboard.h"
#include "wiimote.h"

// TODO
// This KEYBIND macro is a terrible hack
// get rid of it

#define KEYBIND(W, K) if(button_buffer.W != w->buttons.W){ keyboard_key_set(keyboard_key_code(K), button_buffer.W); }

#define BUFFER_LENGTH 255
        

int main(void){

  int res;
  
  res = hid_init();
  if(res == -1){
    // hid_init had an error
    printf("hidapi initialization failed");
    exit(1);
  }

  struct hid_device_info *devs, *cur_dev;

  devs = hid_enumerate(wiimote_vendor_id, wiimote_product_id);
  cur_dev = devs;

  if(!cur_dev){
    printf("No wiimotes found");
  } else {
    printf("Using wiimote %ls\n", cur_dev->serial_number);

    wiimote* w = wiimote_new(cur_dev->serial_number);

    if(w == NULL){
      // couldn't open wiimote
      printf("Fatal: Couldn't open wiimote\n");
    } else {  
    
      unsigned char buffer[BUFFER_LENGTH];

      int bytes_read = wiimote_read(w, buffer, BUFFER_LENGTH);
      
      w->rumble = false;
      wiimote_set_leds(w, LED_1 | LED_2);
     
      wiimote_core_buttons button_buffer;

      wiimote_request_status_report(w);

      while(bytes_read > 0){
        if(buffer[0] != 0x3d){
          // 0x3d is the one report type that doesn't 
          // provide core buttons data.
          // For all other reports, parse core buttons
          // data, and run appropriate responses


          wiimote_parse_core_buttons(&button_buffer, buffer);
          
          KEYBIND(left, 'a');
          KEYBIND(right, 'd');
          KEYBIND(up, 'w');
          KEYBIND(down, 's');
          KEYBIND(A, 'q');
          KEYBIND(plus, 'r');
          KEYBIND(minus, 'e');
   
          if(button_buffer.B != w->buttons.B){
            keyboard_key_set(kVK_Space, button_buffer.B);
          }

          if(button_buffer.one && !w->buttons.one){
            wiimote_request_memory(w, 0, 0x1700, false);
          }

          if(button_buffer.two && !w->buttons.two){
            printf("Writing memory to data.bin\n");
            FILE* f = fopen("data.bin", "w");
            fwrite(w->memory, sizeof(unsigned char), 0x1700, f);
            fclose(f);
          }

          if(button_buffer.home && !w->buttons.home){
            wiimote_request_status_report(w);
            if(w->reporting_mode == 0x31){
              wiimote_set_reporting_mode(w, 0x30, false);
            } else if(w->reporting_mode == 0x30){
              wiimote_set_reporting_mode(w, 0x31, false);
            }
          }

          w->buttons = button_buffer;

        }
        
        wiimote_status_report s;
        
        short acc_x, acc_y, acc_z;
        acc_x = acc_y = acc_z = 0;

        unsigned char memory_size;

        switch(buffer[0]){
          case 0x20: // CB + status report
            //wiimote_status_report s;
            s.lf_byte = buffer[3];
            printf("LEDS: %c%c%c%c\n", 
              s.led1 ? '*' : '.',
              s.led2 ? '*' : '.',
              s.led3 ? '*' : '.',
              s.led4 ? '*' : '.');
            break;
          case 0x21: // CB + Read memory data
            // TODO 
            // Keep an offline copy of the wiimote's memory
            // and update it as data comes in
            // Just need to figure out how to differentiate
            // between the different kinds of reads (memory/register/extension)
            printf("Reading memory at address 0x%02x%02x\n", buffer[4], buffer[5]);
            memory_size = (buffer[3] >> 4) + 1;
            for(int i = 0; i < memory_size; i++){
              w->memory[w->memory_address] = buffer[6+i];
              w->memory_address++;
            }
            if(w->memory_address >= w->memory_end_address){
              w->reading_memory = false;
              printf("Done reading memory\n");
            }
            /*printf("Got %d bytes starting with offset 0x%02x%02x\n", 
               memory_size, buffer[5], buffer[4]);
            for(int i = 0; i < memory_size; i++){
              printf("%02x ", buffer[6 + i]);
            }
            printf("\n");*/
            break;
          case 0x22: // acknowledge output report, maybe report error...
            if(buffer[4]){
              printf("Possible error with report number 0x%02x\n", buffer[3]);
            }
            break;
          case 0x30: // Core buttons
            break;
          case 0x31: // Core buttons + accelerometer
            // TODO double check that this is calculated correctly
            // because there is an asymmetry where down is -100 and up is 88
            acc_x = (buffer[3] << 2) | (buffer[1] & (0x20 | 0x40) >> 5);
            acc_y = (buffer[4] << 2) | (buffer[2] & 0x20 >> 4);
            acc_z = (buffer[5] << 2) | (buffer[2] & 0x40 >> 5);
   
            // TODO calculate calibration values 
            acc_x -= 464;
            acc_y -= 472;
            acc_z -= 476;

            printf("Accelerometer data: \n");
            printf("x %d\n", acc_x);
            printf("y %d\n", acc_y);
            printf("z %d\n", acc_z);
            break;
          case 0x32: // CB + 8 extension bytes
          case 0x33: // CB + acc + 12 IR bytes
          case 0x34: // CB + 19 extension bytes
          case 0x35: // CB + acc + 16 extension bytes
          case 0x36: // CB + 10 IR bytes + 9 extension bytes
          case 0x37: // CB + acc + 10 IR + 9 extension
          case 0x3d: // 21 extension bytes (no core buttons!)
          case 0x3e: // Interleaved CB + acc + 36 IR bytes
          case 0x3f: // same as 0x3e. Def see documentation for these ones
            break;
          default:
            printf("Unrecognized report number 0x%02x\n", buffer[0]);
            break;
        }

        bytes_read = wiimote_read(w, buffer, BUFFER_LENGTH);
      }
    }

    hid_close(w->handle);
    wiimote_free(w);
  }
  
  hid_free_enumeration(devs);

  res = hid_exit();
  if(res == -1){
    // hid_exit had an error
  }
}

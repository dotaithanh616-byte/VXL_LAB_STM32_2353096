#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

typedef enum {
  BTN1 = 0,  // PB0
  BTN2,      // PB1
  BTN3,      // PB2
  NUM_BUTTONS
} button_id_t;

// call once after GPIO init
void buttons_init(void);

void buttons_reading_10ms(void);

uint8_t button_is_pressed(button_id_t id);

uint8_t button_pressed_edge(button_id_t id);      // 1 on 0->1 edge (press)
uint8_t button_released_edge(button_id_t id);     // 1 on 1->0 edge (release)

// long-press (>=1s) latched while held
uint8_t button_is_long_pressed(button_id_t id);   // 1 once held >=1s (stays 1 while still held)

#endif /* INC_INPUT_READING_H_ */

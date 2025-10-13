#include "main.h"
#include "input_reading.h"


#define DURATION_FOR_LONGPRESS_1S   100   // 100 * 10 ms = 1 s
#define BUTTON_IS_PRESSED_LEVEL     GPIO_PIN_RESET   // active-LOW
#define BUTTON_IS_RELEASED_LEVEL    GPIO_PIN_SET


// debounced stable state (logical: 1=pressed, 0=released)
static uint8_t stable_pressed[NUM_BUTTONS];
// previous stable (for reference; edges are latched separately)
static uint8_t stable_prev[NUM_BUTTONS];

// raw debounce buffers (store raw pin level)
static GPIO_PinState dbuf1[NUM_BUTTONS];
static GPIO_PinState dbuf2[NUM_BUTTONS];

// long-press tracking
static uint16_t hold_cnt[NUM_BUTTONS];
static uint8_t  long_pressed[NUM_BUTTONS];

// edge one-shots (latched until read)
static uint8_t pressed_edge_oneshot[NUM_BUTTONS];
static uint8_t released_edge_oneshot[NUM_BUTTONS];

static inline GPIO_PinState read_hw(button_id_t id){
  switch (id){
    case BTN1: return HAL_GPIO_ReadPin(GPIOB, BTN1_Pin);
    case BTN2: return HAL_GPIO_ReadPin(GPIOB, BTN2_Pin);
    default:   return HAL_GPIO_ReadPin(GPIOB, BTN3_Pin);
  }
}

void buttons_init(void){
  for (int i = 0; i < NUM_BUTTONS; ++i){
    dbuf1[i] = dbuf2[i] = BUTTON_IS_RELEASED_LEVEL;
    stable_prev[i] = stable_pressed[i] = 0; // logical released
    hold_cnt[i] = 0;
    long_pressed[i] = 0;
    pressed_edge_oneshot[i] = 0;
    released_edge_oneshot[i] = 0;
  }
}

// Call every 10 ms (e.g., from TIM2 timebase)
void buttons_reading_10ms(void){
  for (int i = 0; i < NUM_BUTTONS; ++i){
    dbuf2[i] = dbuf1[i];
    dbuf1[i] = read_hw((button_id_t)i);

    if (dbuf1[i] == dbuf2[i]){
      uint8_t new_pressed = (dbuf1[i] == BUTTON_IS_PRESSED_LEVEL) ? 1u : 0u;

      if (new_pressed != stable_pressed[i]){
        if (new_pressed){
          pressed_edge_oneshot[i] = 1;
          hold_cnt[i] = 0;
          long_pressed[i] = 0;
        } else {
          released_edge_oneshot[i] = 1;
          hold_cnt[i] = 0;
          long_pressed[i] = 0;
        }
        stable_prev[i]    = stable_pressed[i];
        stable_pressed[i] = new_pressed;
      }

      if (stable_pressed[i]){
        if (hold_cnt[i] < DURATION_FOR_LONGPRESS_1S) {
          hold_cnt[i]++;
          if (hold_cnt[i] >= DURATION_FOR_LONGPRESS_1S){
            long_pressed[i] = 1;
          }
        } else {
          long_pressed[i] = 1;
        }
      }
    }
  }
}

uint8_t button_is_pressed(button_id_t id){
  return (id < NUM_BUTTONS) ? stable_pressed[id] : 0;
}

uint8_t button_is_long_pressed(button_id_t id){
  return (id < NUM_BUTTONS) ? long_pressed[id] : 0;
}

uint8_t button_pressed_edge(button_id_t id){
  if (id >= NUM_BUTTONS) return 0;
  uint8_t v = pressed_edge_oneshot[id];
  pressed_edge_oneshot[id] = 0;
  return v;
}

uint8_t button_released_edge(button_id_t id){
  if (id >= NUM_BUTTONS) return 0;
  uint8_t v = released_edge_oneshot[id];
  released_edge_oneshot[id] = 0;
  return v;
}

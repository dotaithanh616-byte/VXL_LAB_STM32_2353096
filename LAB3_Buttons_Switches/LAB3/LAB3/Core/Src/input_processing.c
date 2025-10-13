#include "main.h"
#include "soft_timers.h"
#include "input_reading.h"
#include "input_processing.h"


#define BTN2_STEP 1

static uint8_t ns_green_snap, ns_yellow_snap, ns_red_snap;
static uint8_t ns_green_edit, ns_yellow_edit, ns_red_edit;

static inline uint8_t tens(uint8_t v){ return v / 10; }
static inline uint8_t ones(uint8_t v){ return v % 10; }

static inline void leds_all_off(void){
  HAL_GPIO_WritePin(GPIOA,
    R1_W_Pin|R2_N_Pin|R3_E_Pin|R4_S_Pin|
    Y1_W_Pin|Y2_N_Pin|Y3_E_Pin|Y4_S_Pin|
    G1_W_Pin|G2_N_Pin|G3_E_Pin|G4_S_Pin, GPIO_PIN_RESET);
}
static inline void we_red_on(void){ HAL_GPIO_WritePin(GPIOA, R1_W_Pin, GPIO_PIN_SET); }
static inline void we_yel_on(void){ HAL_GPIO_WritePin(GPIOA, Y1_W_Pin, GPIO_PIN_SET); }
static inline void we_grn_on(void){ HAL_GPIO_WritePin(GPIOA, G1_W_Pin, GPIO_PIN_SET); }
static inline void ns_red_on(void){ HAL_GPIO_WritePin(GPIOA, R2_N_Pin, GPIO_PIN_SET); }
static inline void ns_yel_on(void){ HAL_GPIO_WritePin(GPIOA, Y2_N_Pin, GPIO_PIN_SET); }
static inline void ns_grn_on(void){ HAL_GPIO_WritePin(GPIOA, G2_N_Pin, GPIO_PIN_SET); }
static inline void ea_red_on(void){ HAL_GPIO_WritePin(GPIOA, R3_E_Pin, GPIO_PIN_SET); }
static inline void ea_yel_on(void){ HAL_GPIO_WritePin(GPIOA, Y3_E_Pin, GPIO_PIN_SET); }
static inline void ea_grn_on(void){ HAL_GPIO_WritePin(GPIOA, G3_E_Pin, GPIO_PIN_SET); }
static inline void so_red_on(void){ HAL_GPIO_WritePin(GPIOA, R4_S_Pin, GPIO_PIN_SET); }
static inline void so_yel_on(void){ HAL_GPIO_WritePin(GPIOA, Y4_S_Pin, GPIO_PIN_SET); }
static inline void so_grn_on(void){ HAL_GPIO_WritePin(GPIOA, G4_S_Pin, GPIO_PIN_SET); }


typedef enum { MODE_1=0, MODE_2, MODE_3, MODE_4 } mode_t;
static mode_t mode = MODE_1;
static mode_t last_mode = 255;

// Editable – Main (North–South) axis
static uint8_t ns_green  = 5;
static uint8_t ns_yellow = 3;
static uint8_t ns_red    = 5;    // = ew_green + ew_yellow

// Derived – Secondary (East–West) axis
static uint8_t ew_green  = 2;
static uint8_t ew_yellow = 3;
static uint8_t ew_red    = 8;    // = ns_green + ns_yellow


typedef enum {
  PH_WE_GREEN = 0,
  PH_WE_YELLOW,
  PH_NS_GREEN,
  PH_NS_YELLOW
} phase_t;

static phase_t phase = PH_WE_GREEN;
static uint8_t  phase_remain = 0;
static uint32_t next_1s_ms   = 0;

static const uint16_t seg_font[10] = {
  SEG0_Pin|SEG1_Pin|SEG2_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin,            // 0
  SEG1_Pin|SEG2_Pin,                                                // 1
  SEG0_Pin|SEG1_Pin|SEG3_Pin|SEG4_Pin|SEG6_Pin,                     // 2
  SEG0_Pin|SEG1_Pin|SEG2_Pin|SEG3_Pin|SEG6_Pin,                     // 3
  SEG1_Pin|SEG2_Pin|SEG5_Pin|SEG6_Pin,                              // 4
  SEG0_Pin|SEG2_Pin|SEG3_Pin|SEG5_Pin|SEG6_Pin,                     // 5
  SEG0_Pin|SEG2_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin|SEG6_Pin,            // 6
  SEG0_Pin|SEG1_Pin|SEG2_Pin,                                       // 7
  SEG0_Pin|SEG1_Pin|SEG2_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin|SEG6_Pin,   // 8
  SEG0_Pin|SEG1_Pin|SEG2_Pin|SEG3_Pin|SEG5_Pin|SEG6_Pin             // 9
};

static inline void seg_write_digit(uint8_t val){
  uint16_t logical = (val <= 9) ? seg_font[val] : 0;
  GPIOB->BSRR = (SEG0_Pin|SEG1_Pin|SEG2_Pin|SEG3_Pin|
                 SEG4_Pin|SEG5_Pin|SEG6_Pin);      // all OFF (set HIGH)
  GPIOB->BSRR = ((uint32_t)logical) << 16;         // selected ON (LOW)
}
static inline void en_all_off(void){
  HAL_GPIO_WritePin(GPIOB, EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin, GPIO_PIN_SET);
}
static inline void en_on(uint8_t i){
  uint16_t pin = (i==0)?EN0_Pin:(i==1)?EN1_Pin:(i==2)?EN2_Pin:EN3_Pin;
  HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_RESET);
}

static void compute_corridor_remaining(uint8_t *we_sec, uint8_t *ns_sec){
    uint8_t we = 0, ns = 0;

    switch (phase){
      case PH_WE_GREEN:
        we = phase_remain;                 // WE active
        ns = ew_yellow + phase_remain;     // NS waiting
        break;
      case PH_WE_YELLOW:
        we = phase_remain;
        ns = phase_remain;
        break;
      case PH_NS_GREEN:
        ns = phase_remain;                 // NS active
        we = ns_yellow + phase_remain;     // WE waiting
        break;
      case PH_NS_YELLOW:
        ns = phase_remain;
        we = phase_remain;
        break;
    }
    if (we > 99) we = 99;
    if (ns > 99) ns = 99;
    *we_sec = we;
    *ns_sec = ns;
}


static void display_mode_update_250ms(uint8_t mode_num, uint8_t value){
  static uint8_t cur = 0;
  static uint32_t next_ms = 0;
  static uint8_t blink = 0;

  if ((int32_t)(sys_ms - next_ms) < 0) return;
  next_ms = sys_ms + 250;
  blink ^= 1;

  leds_all_off();
  if (blink) {
    switch(mode_num){
      case 2: we_red_on(); ns_red_on(); ea_red_on(); so_red_on(); break;
      case 3: we_yel_on(); ns_yel_on(); ea_yel_on(); so_yel_on(); break;
      case 4: we_grn_on(); ns_grn_on(); ea_grn_on(); so_grn_on(); break;
    }
  }

  uint8_t lt=0, lo=mode_num%10;
  uint8_t rt=(value/10)%10, ro=value%10;
  uint8_t val = (cur==0)?lt:(cur==1)?lo:(cur==2)?rt:ro;

  en_all_off();
  seg_write_digit(val);
  en_on(cur);
  cur = (uint8_t)((cur + 1) & 0x03);
}

static void display_update_250ms(void){
  static uint8_t cur = 0;
  static uint32_t next_ms = 0;
  if ((int32_t)(sys_ms - next_ms) < 0) return;
  next_ms = sys_ms + 250;

  uint8_t we_sec, ns_sec;
  compute_corridor_remaining(&we_sec, &ns_sec);

  uint8_t val = 0;
  switch (cur){
    case 0: val = tens(ns_sec); break;
    case 1: val = ones(ns_sec); break;
    case 2: val = tens(we_sec); break;
    case 3: val = ones(we_sec); break;
  }

  en_all_off();
  seg_write_digit(val);
  en_on(cur);
  cur = (uint8_t)((cur + 1) & 0x03);
}

static void traffic_leds_apply(void){
  leds_all_off();
  switch (phase){
    case PH_WE_GREEN:   we_grn_on(); ns_red_on(); ea_grn_on(); so_red_on(); break;
    case PH_WE_YELLOW:  we_yel_on(); ns_red_on(); ea_yel_on(); so_red_on(); break;
    case PH_NS_GREEN:   we_red_on(); ns_grn_on(); ea_red_on(); so_grn_on(); break;
    case PH_NS_YELLOW:  we_red_on(); ns_yel_on(); ea_red_on(); so_yel_on(); break;
  }
}

static void traffic_phase_start(phase_t ph){
  phase = ph;
  switch (phase){
    case PH_WE_GREEN:   phase_remain = ew_green;  break;
    case PH_WE_YELLOW:  phase_remain = ew_yellow; break;
    case PH_NS_GREEN:   phase_remain = ns_green;  break;
    case PH_NS_YELLOW:  phase_remain = ns_yellow; break;
  }
  next_1s_ms = sys_ms + 1000;
  traffic_leds_apply();
}

static void traffic_tick_1s(void){
  if ((int32_t)(sys_ms - next_1s_ms) < 0) return;
  next_1s_ms += 1000;

  if (phase_remain > 0) phase_remain--;
  if (phase_remain == 0){
    switch (phase){
      case PH_WE_GREEN:   traffic_phase_start(PH_WE_YELLOW);  break;
      case PH_WE_YELLOW:  traffic_phase_start(PH_NS_GREEN);   break;
      case PH_NS_GREEN:   traffic_phase_start(PH_NS_YELLOW);  break;
      case PH_NS_YELLOW:  traffic_phase_start(PH_WE_GREEN);   break;
    }
  }
}

static void recalc_times(void){
  ns_red = ew_green + ew_yellow;
  ew_red = ns_green + ns_yellow;
}

void fsm_mode_processing(void){
//  // --- BTN1 cycles modes ---
//  if (button_pressed_edge(0)){
//    mode = (mode == MODE_4) ? MODE_1 : (mode_t)(mode + 1);
//  }

  if (mode != last_mode){
    last_mode = mode;
    if (mode == MODE_1){
      traffic_phase_start(PH_WE_GREEN);
    } else {
        leds_all_off();
        en_all_off();

        ns_green_snap  = ns_green;
        ns_yellow_snap = ns_yellow;
        ns_red_snap    = ns_red;

        ns_green_edit  = ns_green;
        ns_yellow_edit = ns_yellow;
        ns_red_edit    = ns_red;

        switch (mode){
          case MODE_2: we_red_on(); ns_red_on(); ea_red_on(); so_red_on(); break;
          case MODE_3: we_yel_on(); ns_yel_on(); ea_yel_on(); so_yel_on(); break;
          case MODE_4: we_grn_on(); ns_grn_on(); ea_grn_on(); so_grn_on(); break;
          break;
          default:
          break;
        }
      }
  }

  switch (mode){
  case MODE_1:
    if (button_pressed_edge(BTN1)) { mode = MODE_2; last_mode = 255; break; }
    traffic_tick_1s();
    display_update_250ms();
    break;

    case MODE_2:
      if (button_pressed_edge(BTN2)) {
    	  ns_red_edit = (ns_red_edit + BTN2_STEP);
        if (ns_red_edit > 99) ns_red_edit = 1;
      }
      if (button_pressed_edge(BTN3)) {
        ns_red = ns_red_edit;
        ew_green = (ns_red > ew_yellow) ? (ns_red - ew_yellow) : 1;
        recalc_times();
        mode = MODE_1; last_mode = 255; break;
      }
      if (button_pressed_edge(BTN1)) {
        ns_green  = ns_green_snap;
        ns_yellow = ns_yellow_snap;
        ns_red    = ns_red_snap;
        recalc_times();
        mode = MODE_3; last_mode = 255; break;
      }

      display_mode_update_250ms(2, ns_red_edit);
      break;

    case MODE_3:
      if (button_pressed_edge(BTN2)) {
    	  ns_yellow_edit = (ns_yellow_edit + BTN2_STEP);
        if (ns_yellow_edit > 99) ns_yellow_edit = 1;
      }
      if (button_pressed_edge(BTN3)) {
        ns_yellow = ns_yellow_edit;
        recalc_times();
        mode = MODE_1; last_mode = 255; break;
      }
      if (button_pressed_edge(BTN1)) {
        ns_green  = ns_green_snap;
        ns_yellow = ns_yellow_snap;
        ns_red    = ns_red_snap;
        recalc_times();
        mode = MODE_4; last_mode = 255; break;
      }

      display_mode_update_250ms(3, ns_yellow_edit);
      break;

    case MODE_4:
      if (button_pressed_edge(BTN2)) {
    	  ns_green_edit = (ns_green_edit + BTN2_STEP);
        if (ns_green_edit > 99) ns_green_edit = 1;
      }
      if (button_pressed_edge(BTN3)) {
        ns_green = ns_green_edit;
        recalc_times();
        mode = MODE_1; last_mode = 255; break;
      }
      if (button_pressed_edge(BTN1)) {
        ns_green  = ns_green_snap;
        ns_yellow = ns_yellow_snap;
        ns_red    = ns_red_snap;
        recalc_times();
        mode = MODE_1; last_mode = 255; break;
      }

      display_mode_update_250ms(4, ns_green_edit);
      break;
  }
}

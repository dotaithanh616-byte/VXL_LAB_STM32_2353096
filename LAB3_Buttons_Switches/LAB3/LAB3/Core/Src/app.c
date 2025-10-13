#include "app.h"
#include "main.h"
#include "soft_timers.h"

static soft_timer_t t500;
static uint8_t red_on = 0;

void app_init(void){
  timer_start_ms(&t500, 500);
}

void app_tick(void){
  if (timer_expired(&t500)){
    timer_start_ms(&t500, 500);
    red_on ^= 1;
    HAL_GPIO_WritePin(GPIOA, R1_W_Pin|R2_N_Pin|R3_E_Pin|R4_S_Pin,
                      red_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

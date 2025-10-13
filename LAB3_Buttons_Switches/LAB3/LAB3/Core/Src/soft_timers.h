
#ifndef SRC_SOFT_TIMERS_H_
#define SRC_SOFT_TIMERS_H_

#include <stdint.h>

#ifndef TICK_MS
#define TICK_MS  2u
#endif

extern volatile uint32_t sys_ms;     // global "now" in ms counter
static inline uint32_t now_ms(void){ return sys_ms; }

typedef struct {
    uint32_t t_expire;               // absolute expiration time (ms)
} soft_timer_t;


static inline void timer_start_ms(soft_timer_t *t, uint32_t delay_ms){
    t->t_expire = now_ms() + delay_ms;
}

static inline uint8_t timer_expired(soft_timer_t *t){
    return (int32_t)(now_ms() - t->t_expire) >= 0;
}

#endif /* SRC_SOFT_TIMERS_H_ */

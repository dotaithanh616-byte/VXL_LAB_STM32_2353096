#ifndef UART_COMMUNICATION_FSM_H
#define UART_COMMUNICATION_FSM_H

#include <stdint.h>

// Call every 10–20 ms from the scheduler
void uart_communication_fsm(void);

// Producer for ADC value (call from a periodic task, e.g., 100–200 ms)
void set_latest_adc(uint16_t v);

#endif

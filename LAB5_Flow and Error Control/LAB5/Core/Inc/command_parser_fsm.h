#ifndef COMMAND_PARSER_FSM_H
#define COMMAND_PARSER_FSM_H

#include <stdint.h>

// Public API: call this every 10ms from the scheduler
void command_parser_fsm(void);

// Output flags read by the communication FSM (auto-cleared when consumed)
uint8_t cmd_take_RST(void);  // returns 1 if a new !RST# was parsed
uint8_t cmd_take_OK(void);   // returns 1 if a new !OK# was parsed

// RX ring-buffer bootstrap (start the first interrupt-driven byte reception)
void uart_rx_init(void);

#endif

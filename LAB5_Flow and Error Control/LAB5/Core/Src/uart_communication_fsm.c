#include "uart_communication_fsm.h"
#include "command_parser_fsm.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>

extern UART_HandleTypeDef huart2;

typedef enum {
    UC_IDLE = 0,
    UC_SEND_ADC,
    UC_WAIT_ACK
} uc_state_t;

static uc_state_t state   = UC_IDLE;
static uint32_t   tmo_ms  = 0U;      // countdown in milliseconds
static uint16_t   last_adc = 0U;

#ifndef SCH_TICK_MS
#define SCH_TICK_MS      10U
#endif

#define ACK_TIMEOUT_MS 3000U

// updated by ADC task
void set_latest_adc(uint16_t v)
{
    last_adc = v;
}

//  helper to send a packet like "!ADC=1234#"
static void send_adc_packet(uint16_t val)
{
    char pkt[20];
    int n = snprintf(pkt, sizeof(pkt), "!ADC=%u#", (unsigned)val);

    if (n > 0) {
        (void)HAL_UART_Transmit(&huart2, (uint8_t*)pkt, (uint16_t)n, 50U);
    }
}

void uart_communication_fsm(void)
{
    // Check for incoming commands parsed by parser FSM

    // Only accept RST when idle
    if ((state == UC_IDLE) && cmd_take_RST()) {
        state = UC_SEND_ADC;
    }

    // While waiting for ACK, check for OK
    if ((state == UC_WAIT_ACK) && cmd_take_OK()) {
        // ACK received -> complete exchange
        state  = UC_IDLE;
        tmo_ms = 0U;
        return;
    }

    // Act by state
    switch (state) {
    case UC_IDLE:
        // Nothing to do; wait for !RST#
        break;

    case UC_SEND_ADC:
        send_adc_packet(last_adc);
        // Start timeout window (resend if expires)
        tmo_ms = ACK_TIMEOUT_MS;
        state  = UC_WAIT_ACK;
        break;

    case UC_WAIT_ACK:
        if (tmo_ms > SCH_TICK_MS) {
            tmo_ms -= SCH_TICK_MS;
        } else {
            // Timeout -> resend and restart timeout window
            send_adc_packet(last_adc);
            tmo_ms = ACK_TIMEOUT_MS;
        }
        break;

    default:
        state  = UC_IDLE;
        tmo_ms = 0U;
        break;
    }
}

#include "command_parser_fsm.h"
#include "stm32f1xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart2;

// --- UART RX ring buffer fed by ISR ---
#define RX_BUF_SIZE 64

static volatile uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;   // index where ISR writes next byte
static volatile uint16_t rx_tail = 0;   // index where FSM reads next byte

// single-byte buffer for HAL_UART_Receive_IT
static uint8_t rx_byte;

// --- Command parser FSM ---
typedef enum {
    CP_IDLE,
    CP_IN_FRAME
} cp_state_t;

static cp_state_t cp_state = CP_IDLE;

#define CMD_BUF_SIZE 16
static char    cmd_buf[CMD_BUF_SIZE];
static uint8_t cmd_len = 0;

// one-shot flags consumed by the comm FSM
static volatile uint8_t flag_RST = 0;
static volatile uint8_t flag_OK  = 0;

// --- ring-buffer helper ---
static int rb_pop(uint8_t *out)
{
    if (rx_tail == rx_head) {
        return 0;   // empty
    }
    *out = rx_buf[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1U) % RX_BUF_SIZE);
    return 1;
}

void uart_rx_init(void)
{
    // Arm first interrupt receive
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

// ISR callback: push received byte into ring buffer and re-arm
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        uint16_t next = (uint16_t)((rx_head + 1U) % RX_BUF_SIZE);

        // only store if buffer is not full
        if (next != rx_tail) {
            rx_buf[rx_head] = rx_byte;
            rx_head = next;
        }
        // re-arm for next byte
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// Expose one-shot flags
uint8_t cmd_take_RST(void)
{
    uint8_t f = flag_RST;
    flag_RST = 0;
    return f;
}

uint8_t cmd_take_OK(void)
{
    uint8_t f = flag_OK;
    flag_OK = 0;
    return f;
}

// Parse bytes into commands like !RST# and !OK#
void command_parser_fsm(void)
{
    uint8_t b;

    while (rb_pop(&b)) {
        switch (cp_state) {
        case CP_IDLE:
            if (b == '!') {
                cp_state = CP_IN_FRAME;
                cmd_len  = 0;
            }
            break;

        case CP_IN_FRAME:
            if (b == '#') {
                // Terminate and evaluate
                cmd_buf[cmd_len] = '\0';

                if (strcmp(cmd_buf, "RST") == 0) {
                    flag_RST = 1;
                } else if (strcmp(cmd_buf, "OK") == 0) {
                    flag_OK = 1;
                }

                // reset parser
                cp_state = CP_IDLE;
                cmd_len  = 0;
            } else {
                if (cmd_len < (CMD_BUF_SIZE - 1U)) {
                    cmd_buf[cmd_len++] = (char)b;
                } else {
                    // overflow -> drop frame and reset
                    cp_state = CP_IDLE;
                    cmd_len  = 0;
                }
            }
            break;

        default:
            cp_state = CP_IDLE;
            cmd_len  = 0;
            break;
        }
    }
}

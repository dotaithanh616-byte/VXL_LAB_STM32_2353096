// === scheduler.h additions ===
#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tick config (10 ms per TIM2 interrupt)
#ifndef SCH_TICK_MS
#define SCH_TICK_MS   (10U)
#endif

// Max tasks
#ifndef SCH_MAX_TASKS
#define SCH_MAX_TASKS (16U)
#endif

// ---- Error codes ----
#define ERROR_SCH_OK                    0U
#define ERROR_SCH_TOO_MANY_TASKS        1U
#define ERROR_SCH_CANNOT_DELETE_TASK    2U
#define ERROR_SCH_NULL_FUNCTION         3U

extern volatile uint8_t Error_code_G;

// ---- Task type ----
typedef struct {
    void     (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint8_t  RunMe;
    uint32_t TaskID;
} sTask;

// ---- API ----
void     SCH_Init(void);
void     SCH_Update(void);
void     SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Go_To_Sleep(void);   // may call __WFI()
void     SCH_Report_Status(void); // error hook
uint32_t SCH_TimeMs(void);

#ifdef __cplusplus
}
#endif
#endif /* __SCHEDULER_H */

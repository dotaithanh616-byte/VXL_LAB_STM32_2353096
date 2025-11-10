// scheduler.h
#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Tick size (ms) driven by TIM2 interrupt
#ifndef SCH_TICK_MS
#define SCH_TICK_MS   (10U)
#endif

// Max number of tasks
#ifndef SCH_MAX_TASKS
#define SCH_MAX_TASKS (16U)
#endif

// Error codes
#define ERROR_SCH_OK                    0U
#define ERROR_SCH_TOO_MANY_TASKS        1U
#define ERROR_SCH_CANNOT_DELETE_TASK    2U
#define ERROR_SCH_NULL_FUNCTION         3U

extern volatile uint8_t Error_code_G;

// Core API
void     SCH_Init(void);                         // init tables and queue
void     SCH_Update(void);                       // called from TIM2 ISR: O(1)
void     SCH_Dispatch_Tasks(void);               // run due tasks, reschedule
uint32_t SCH_Add_Task(void (*pTask)(void),       // returns task id or SCH_MAX_TASKS on error
                      uint32_t delayTicks,
                      uint32_t periodTicks);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Go_To_Sleep(void);                  // weak; default __WFI()
void     SCH_Report_Status(void);                // weak; error hook
uint32_t SCH_TimeMs(void);                       // coarse uptime (ms)

#ifdef __cplusplus
}
#endif
#endif

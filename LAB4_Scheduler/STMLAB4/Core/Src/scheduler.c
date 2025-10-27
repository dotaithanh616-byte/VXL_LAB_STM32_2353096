#include "scheduler.h"

#ifndef SCH_TICK_MS
#define SCH_TICK_MS   (10U)
#endif
#ifndef SCH_MAX_TASKS
#define SCH_MAX_TASKS (16U)
#endif

// ===== Task table =====
typedef struct {
    void     (*pTask)(void);
    uint32_t Period;     // ticks between runs (0 = one-shot)
    uint8_t  RunMe;      // due count
    uint8_t  used;       // slot in use
} TCB;

static TCB T[SCH_MAX_TASKS];

// ===== Delta queue (relative delays) =====
static int16_t  nextIdx[SCH_MAX_TASKS];
static uint32_t relDelay[SCH_MAX_TASKS];
static int16_t  qHead = -1;

static volatile uint32_t sch_ticks = 0;
volatile uint8_t Error_code_G = ERROR_SCH_OK;

// ---- internal helpers (non-ISR) ----
static void q_reset(void) {
    qHead = -1;
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        nextIdx[i] = -1;
        relDelay[i] = 0;
    }
}

static void q_insert(int16_t id, uint32_t delayTicks) {
    // Insert task id with absolute delay into delta queue as relative delay
    int16_t prev = -1, cur = qHead;
    uint32_t d = delayTicks;

    while (cur != -1 && d >= relDelay[cur]) {
        d -= relDelay[cur];
        prev = cur;
        cur  = nextIdx[cur];
    }

    // Insert id between prev and cur; set its relative delay
    relDelay[id] = d;
    nextIdx[id]  = cur;

    if (prev == -1) {
        if (qHead != -1) relDelay[qHead] -= d;
        qHead = id;
    } else {
        nextIdx[prev] = id;
        if (cur != -1) relDelay[cur] -= d;
    }
}

static int16_t q_pop_head(void) {
    if (qHead == -1) return -1;
    int16_t id = qHead;
    qHead = nextIdx[id];
    nextIdx[id] = -1;
    return id;
}

void SCH_Init(void) {
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        T[i].pTask  = NULL;
        T[i].Period = 0;
        T[i].RunMe  = 0;
        T[i].used   = 0;
        nextIdx[i]  = -1;
        relDelay[i] = 0;
    }
    q_reset();
    Error_code_G = ERROR_SCH_OK;
}

void SCH_Update(void) {
    sch_ticks++;

    if (qHead == -1) return;

    if (relDelay[qHead] > 0) {
        relDelay[qHead]--;            // 1 tick passed
    }

    while (qHead != -1 && relDelay[qHead] == 0) {
        // head is due now
        int16_t id = q_pop_head();
        if (id >= 0 && T[id].used && T[id].pTask) {
            T[id].RunMe++;           // mark due; run in dispatcher
        }
    }
}

uint32_t SCH_TimeMs(void) {
    return sch_ticks * SCH_TICK_MS;
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    if (pFunction == NULL) {
        Error_code_G = ERROR_SCH_NULL_FUNCTION;
        return SCH_MAX_TASKS;
    }
    uint32_t id = 0;
    while (id < SCH_MAX_TASKS && T[id].used) id++;
    if (id >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS;
    }
    T[id].pTask  = pFunction;
    T[id].Period = PERIOD;
    T[id].RunMe  = 0;
    T[id].used   = 1;

    q_insert((int16_t)id, DELAY);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID >= SCH_MAX_TASKS || !T[taskID].used) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return 1U;
    }
    int16_t cur = qHead, prev = -1;
    while (cur != -1) {
        if ((uint32_t)cur == taskID) {
            int16_t nxt = nextIdx[cur];
            if (prev == -1) {
                if (nxt != -1) relDelay[nxt] += relDelay[cur];
                qHead = nxt;
            } else {
                nextIdx[prev] = nxt;
                if (nxt != -1) relDelay[nxt] += relDelay[cur];
            }
            nextIdx[cur]  = -1;
            relDelay[cur] = 0;
            break;
        }
        prev = cur;
        cur  = nextIdx[cur];
    }

    T[taskID].pTask  = NULL;
    T[taskID].Period = 0;
    T[taskID].RunMe  = 0;
    T[taskID].used   = 0;
    return 0U;
}

void SCH_Dispatch_Tasks(void) {
    for (uint32_t id = 0; id < SCH_MAX_TASKS; id++) {
        while (T[id].used && T[id].RunMe > 0) {
            T[id].RunMe--;
            void (*fn)(void) = T[id].pTask;
            fn(); // run task

            if (T[id].used && T[id].Period > 0) {
                q_insert((int16_t)id, T[id].Period);
            } else if (T[id].used && T[id].Period == 0) {
                (void)SCH_Delete_Task(id);
            }
        }
    }
    SCH_Report_Status();
    SCH_Go_To_Sleep();
}

__attribute__((weak)) void SCH_Report_Status(void) {  }
__attribute__((weak)) void SCH_Go_To_Sleep(void)    { __WFI(); }

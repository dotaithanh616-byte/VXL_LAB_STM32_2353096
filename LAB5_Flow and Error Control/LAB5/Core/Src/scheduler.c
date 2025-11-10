// scheduler.c
#include "scheduler.h"
#include <string.h>

typedef struct {
    void     (*pTask)(void);
    uint32_t Period;        // in ticks; 0 = one-shot
    uint8_t  RunMe;         // counts pending runs (guarded against overflow)
    uint8_t  used;          // slot in use
} TCB;

static TCB T[SCH_MAX_TASKS];

/* Delta queue links and relative delays */
static int16_t  nextIdx[SCH_MAX_TASKS];
static uint32_t relDelay[SCH_MAX_TASKS];
static int16_t  qHead = -1;                 // head of delta queue (-1 = empty)

/* List of due tasks (ready to run), built by ISR, consumed by dispatcher */
static int16_t dueHead = -1;
static int16_t dueNext[SCH_MAX_TASKS];

/* Uptime tick counter (ticks of SCH_TICK_MS) */
static volatile uint32_t g_tickCount = 0;

/* Public error code (see scheduler.h) */
volatile uint8_t Error_code_G = ERROR_SCH_OK;

/* -------- IRQ helpers: tiny critical sections for non-ISR queue edits ------- */
static inline uint32_t sch_irq_lock(void){
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}
static inline void sch_irq_unlock(uint32_t primask){
    if (!primask) __enable_irq();
}

/* ------------------------ Internal utility functions ----------------------- */

/* Clear all internal tables */
static void clear_tables(void){
    memset(T, 0, sizeof(T));
    for (uint32_t i = 0; i < SCH_MAX_TASKS; ++i){
        nextIdx[i] = -1;
        relDelay[i] = 0;
        dueNext[i]  = -1;
    }
    qHead   = -1;
    dueHead = -1;
}

/* Insert task id into delta queue at absolute 'delay' ticks from now.
 * Complexity: O(k) where k is number of nodes scanned (NOT in ISR).
 */
static void q_insert(int16_t id, uint32_t delay){
    // ensure node is not already linked
    nextIdx[id] = -1;
    relDelay[id] = delay;

    if (qHead < 0){
        qHead = id;
        return;
    }

    /* Walk the queue, subtracting cumulative delta until insertion point. */
    int16_t prev = -1;
    int16_t cur  = qHead;
    while (cur >= 0 && relDelay[id] > relDelay[cur]){
        relDelay[id] -= relDelay[cur];
        prev = cur;
        cur  = nextIdx[cur];
    }

    if (prev < 0){
        // Insert at head
        nextIdx[id] = qHead;
        qHead = id;
    } else {
        // Insert after 'prev'
        nextIdx[id] = cur;
        nextIdx[prev] = id;
    }

    // If we inserted before 'cur', adjust the next node's relative delay
    if (cur >= 0){
        if (relDelay[cur] >= relDelay[id]) {
            relDelay[cur] -= relDelay[id];
        } else {
            // should not happen, but guard against underflow
            relDelay[cur] = 0;
        }
    }
}

/* Remove a specific id from the delta queue if present.
 * Adjust neighbor relative delays accordingly.
 * Returns 1 if removed, 0 if not found.
 * Complexity: O(k) (NOT in ISR).
 */
static uint8_t q_remove_by_id(int16_t id){
    if (qHead < 0) return 0;

    int16_t prev = -1;
    int16_t cur  = qHead;
    while (cur >= 0){
        if (cur == id){
            // Found it: unlink
            int16_t nxt = nextIdx[cur];
            if (prev < 0){
                // Removing head
                qHead = nxt;
            } else {
                nextIdx[prev] = nxt;
            }
            // Add the removed node's remaining delay to its successor
            if (nxt >= 0){
                relDelay[nxt] += relDelay[cur];
            }
            nextIdx[cur] = -1;
            relDelay[cur] = 0;
            return 1;
        }
        prev = cur;
        cur  = nextIdx[cur];
    }
    return 0;
}

/* Push an id onto the 'due list' (LIFO). Called from ISR only. */
static inline void due_push_from_isr(int16_t id){
    dueNext[id] = dueHead;
    dueHead     = id;
}

/* Pop one id from due list. Needs locking (ISR may push concurrently). */
static int16_t due_pop_locked(void){
    if (dueHead < 0) return -1;
    int16_t id = dueHead;
    dueHead = dueNext[id];
    dueNext[id] = -1;
    return id;
}

/* ------------------------------ API functions ------------------------------ */

void SCH_Init(void){
    clear_tables();
    Error_code_G = ERROR_SCH_OK;
}

/* Called from timer ISR (e.g., TIM2) every SCH_TICK_MS. Keep it tiny. */
void SCH_Update(void){
    g_tickCount++;

    // Decrement only the head's relative delay
    if (qHead >= 0){
        if (relDelay[qHead] > 0){
            relDelay[qHead]--;
        }

        // Pop all matured nodes (relDelay == 0) and push to due list
        // This loop runs only for tasks due *now*, which is typically small.
        while (qHead >= 0 && relDelay[qHead] == 0){
            int16_t id = qHead;
            qHead = nextIdx[id];
            nextIdx[id] = -1;

            // Count pending runs, guard overflow
            if (T[id].RunMe != 0xFF) T[id].RunMe++;

            // Hand off to dispatcher via due list
            due_push_from_isr(id);
        }
    }
}

/* Run due tasks (non-ISR), and reschedule periodic ones. */
void SCH_Dispatch_Tasks(void){
    while (1){
        // Pop at most one at a time under lock (in case ISR pushes concurrently)
        uint32_t key = sch_irq_lock();
        int16_t id = due_pop_locked();
        sch_irq_unlock(key);

        if (id < 0) break; // nothing more to run

        // If task was deleted after being queued, skip execution
        if (!T[id].used){
            T[id].RunMe = 0;
            continue;
        }

        // Consume one pending run
        if (T[id].RunMe > 0) T[id].RunMe--;

        // Execute outside the critical section
        void (*fn)(void) = T[id].pTask;
        if (fn){
            fn();
        }

        // Periodic: reinsert into delta queue
        if (T[id].used && T[id].Period > 0){
            uint32_t key2 = sch_irq_lock();
            q_insert(id, T[id].Period);
            sch_irq_unlock(key2);
        } else {
            // One-shot: free the slot
            T[id].used  = 0;
            T[id].pTask = NULL;
            T[id].Period= 0;
            T[id].RunMe = 0;
        }
    }

    SCH_Report_Status();

    // Power-save hint (no-op by default)
    SCH_Go_To_Sleep();
}

/* Add a task. delayTicks = initial delay; periodTicks = 0 for one-shot. */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t delayTicks, uint32_t periodTicks){
    if (pFunction == NULL){
        Error_code_G = ERROR_SCH_NULL_FUNCTION;
        return SCH_MAX_TASKS;
    }

    // Find a free slot
    uint32_t id = 0;
    while (id < SCH_MAX_TASKS && T[id].used) id++;
    if (id >= SCH_MAX_TASKS){
        Error_code_G = ERROR_SCH_TOO_MANY_TASKS;
        return SCH_MAX_TASKS;
    }

    // Initialize TCB
    T[id].pTask  = pFunction;
    T[id].Period = periodTicks;
    T[id].RunMe  = 0;
    T[id].used   = 1;

    // Insert into delta queue under lock (non-ISR)
    uint32_t key = sch_irq_lock();
    q_insert((int16_t)id, delayTicks);
    sch_irq_unlock(key);

    return id;
}

/* Delete a task by id.
 * Returns 1 on success (slot cleared), 0 on failure (id invalid or not used).
 */
uint8_t SCH_Delete_Task(uint32_t taskID){
    if (taskID >= SCH_MAX_TASKS) {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return 0;
    }
    if (!T[taskID].used){
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return 0;
    }

    // Remove from delta queue if present, under lock
    uint32_t key = sch_irq_lock();
    (void)q_remove_by_id((int16_t)taskID);

    // mark unused; dispatcher will skip execution if it pops it later.
    T[taskID].used   = 0;
    T[taskID].pTask  = NULL;
    T[taskID].Period = 0;
    T[taskID].RunMe  = 0;
    sch_irq_unlock(key);

    return 1;
}


__weak void SCH_Go_To_Sleep(void){
}

__weak void SCH_Report_Status(void){
}

/* Return coarse uptime in milliseconds.*/
uint32_t SCH_TimeMs(void){
    uint32_t ticks = g_tickCount; // single 32-bit read is atomic on Cortex-M3
    return ticks * SCH_TICK_MS;
}

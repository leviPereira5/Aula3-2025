#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"
#include "msg.h"

#define NUM_MLFQ_LEVELS 3

typedef struct {
    queue_t levels[NUM_MLFQ_LEVELS];
    uint32_t quanta[NUM_MLFQ_LEVELS];
} mlfq_ready_t;

/**
 * @brief Multi-Level Feedback Queue (MLFQ) scheduling algorithm.
 *
 * Implements MLFQ with 3 levels:
 * - Level 0: High priority, short quantum (8 ms)
 * - Level 1: Medium priority, medium quantum (16 ms)
 * - Level 2: Low priority, long quantum (non-preemptive effectively)
 *
 * New tasks or post-I/O start at level 0.
 * Exhausted quantum demotes to next level.
 * Selects from highest non-empty level (FIFO within level).
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the MLFQ ready queues.
 * @param cpu_task Double pointer to the currently running task.
 */
void mlfq_scheduler(uint32_t current_time_ms, mlfq_ready_t *rq, pcb_t **cpu_task);

#endif // MLFQ_H
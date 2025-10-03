#include "mlfq.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void mlfq_scheduler(uint32_t current_time_ms, mlfq_ready_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        // Accumulate elapsed time for the current burst
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished its CPU burst
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            // Do not free here; main loop will re-enqueue to command_queue
            *cpu_task = NULL;
            return;
        }

        // Check if quantum exceeded for preemption
        uint32_t slice_elapsed_ms = current_time_ms - (*cpu_task)->slice_start_ms + TICKS_MS;  // Approximate including current tick
        uint8_t level = (*cpu_task)->level;
        if (slice_elapsed_ms >= rq->quanta[level]) {
            // Preempt: demote to next level if possible
            (*cpu_task)->slice_start_ms = current_time_ms;  // Reset slice for potential re-run
            if (level < NUM_MLFQ_LEVELS - 1) {
                (*cpu_task)->level = level + 1;
            }
            // Re-enqueue to the appropriate level
            enqueue_pcb(&rq->levels[(*cpu_task)->level], *cpu_task);
            *cpu_task = NULL;
            return;
        }
    }

    // CPU is idle: select next task from highest priority non-empty level
    for (int l = 0; l < NUM_MLFQ_LEVELS; l++) {
        if (rq->levels[l].head != NULL) {
            *cpu_task = dequeue_pcb(&rq->levels[l]);
            if (*cpu_task) {
                (*cpu_task)->slice_start_ms = current_time_ms;
                break;
            }
        }
    }
}
#include "msg.h"
#include "queue.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static pcb_t* find_shortest_job(queue_t *rq) { // essa funcao serve para procurar o processo mais curto que está na fila
    queue_elem_t *curr = rq->head;
    queue_elem_t *shortest_elem = curr;

    if (!curr) return NULL;

    while (curr != NULL) {
        if (curr->pcb->time_ms < shortest_elem->pcb->time_ms) {
            shortest_elem = curr;
        }
        curr = curr->next;
    }

    // remove o mais curto da fila
    queue_elem_t *removed = remove_queue_elem(rq, shortest_elem);
    if (!removed) return NULL;

    pcb_t *shortest = removed->pcb;
    free(removed);
    return shortest;
}

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            free(*cpu_task);
            *cpu_task = NULL;
        }
    }

    if (*cpu_task == NULL && rq->head != NULL) {
        *cpu_task = find_shortest_job(rq);
    }
}

#include <stdint.h>
#include "queue.h"
#ifndef SJF_H
#define SJF_H

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif

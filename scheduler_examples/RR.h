#ifndef RR_H
#define RR_H


#include "msg.h"
#include "queue.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);



#endif //RR_H

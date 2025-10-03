#ifndef QUEUE_H
#define QUEUE_H
#include <stdint.h>

typedef enum  {
    TASK_COMMAND = 0,   // Task has connected and is waiting for instructions
    TASK_BLOCKED,       // Task is blocked (waiting/IO wait)
    TASK_RUNNING,       // Task is in the ready queue or currently running
    TASK_STOPPED,       // Task has finished execution (sent DONE), waiting for more messages
    TASK_TERMINATED,    // Task has been terminated and will be removed
} task_status_en;

// Define the Process Control Block (PCB) structure
typedef struct pcb_st{
    int32_t pid;                   // Process ID
    task_status_en status;         // Current status of the task defined by the pcb
    uint32_t time_ms;              // Time requested by application in milliseconds
    uint32_t ellapsed_time_ms;     // Time ellapsed since start in milliseconds
    uint32_t slice_start_ms;       // Time when the current time slice started
    uint32_t sockfd;               // Socket file descriptor for communication with the application
    uint8_t level;                 // Current MLFQ level (0 = highest priority)
} pcb_t;

// Define singly linked list elements
typedef struct queue_elem_st queue_elem_t;
typedef struct queue_elem_st {
    pcb_t *pcb;
    queue_elem_t *next;
} queue_elem_t;

// Define the queue structure
typedef struct queue_st  {
    queue_elem_t* head;
    queue_elem_t* tail;
} queue_t;

pcb_t *new_pcb(int32_t pid, uint32_t sockfd, uint32_t time_ms);
int enqueue_pcb(queue_t* q, pcb_t* task);
pcb_t* dequeue_pcb(queue_t* q);
queue_elem_t *remove_queue_elem(queue_t* q, queue_elem_t* elem);

#endif //QUEUE_H
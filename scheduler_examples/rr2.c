#include "rr2.h"
#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include <unistd.h>
#include "debug.h"

//pcb_t = processo (estrutura com informações do processo)
//queue_t = fila de processos prontos
//sjf_scheduler(...) = outro escalonador (não usado aqui, apenas referência)
//TICKS_MS = unidade de tempo simulada

/**
 * Escalonador Round-Robin (RR)
 *
 * O Round-Robin (RR) é um escalonador preemptivo usado em sistemas operativos.
 * A ideia é dar a cada processo um "pedaço de tempo" (quantum) para executar.
 * Se ele não terminar dentro desse tempo, é interrompido e devolvido ao final
 * da fila de prontos. Isso garante justiça, já que todos os processos têm a
 * oportunidade de usar a CPU em turnos.
 *
 * Passo a passo da função rr_scheduler:
 *
 * 1. Contabilização de tempo:
 *    - A cada chamada do escalonador, simula-se a passagem de TICKS_MS (1 ms).
 *    - Esse tempo é adicionado tanto ao tempo total de execução do processo
 *      (ellapsed_time_ms) quanto ao contador do quantum (quantum_counter).
 *
 * 2. Se o processo terminou (ellapsed_time_ms >= time_ms):
 *    - Envia mensagem ao processo notificando que ele terminou (PROCESS_REQUEST_DONE).
 *    - Liberta a memória do PCB.
 *    - A CPU fica livre (*cpu_task = NULL).
 *    - Reinicia o contador de quantum.
 *
 * 3. Se o processo não terminou, mas gastou todo o quantum (quantum_counter >= QUANTUM_MS):
 *    - O processo é removido da CPU e colocado no final da fila de prontos (enqueue_pcb).
 *    - A CPU fica livre.
 *    - Reinicia o contador de quantum.
 *    - Assim, outro processo terá chance de executar no próximo ciclo.
 *
 * 4. Se a CPU está livre (*cpu_task == NULL) e existe processo na fila:
 *    - O escalonador pega o próximo processo da fila (dequeue_pcb).
 *    - Coloca-o na CPU para ser executado nos próximos ticks.
 */

#define QUANTUM_MS 500
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    static uint32_t quantum_counter = 0; // Tracks time spent in current time slice

    // Handle task currently on CPU
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS; // Update total execution time
        quantum_counter += TICKS_MS; // Update time in current quantum

        // Log execution time for debugging
        DBG("Process %d: ellapsed_time_ms=%u, time_ms=%u\n",
            (*cpu_task)->pid, (*cpu_task)->ellapsed_time_ms, (*cpu_task)->time_ms);

        // Case 1: Task has completed its required execution time
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Send DONE message
            msg_t msg = {.pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            } else {
                DBG("Successfully sent DONE to process %d\n", (*cpu_task)->pid);
            }
            DBG("Process %d finished, sending DONE\n", (*cpu_task)->pid);

            // Free the completed task
            free(*cpu_task);
            *cpu_task = NULL;
            quantum_counter = 0; // Reset quantum counter
        }
        // Case 2: Quantum expired but task not finished
        else if (quantum_counter >= QUANTUM_MS) {
            DBG("Process %d preempted after 500ms\n", (*cpu_task)->pid);
            (*cpu_task)->status = TASK_RUNNING;
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;
            quantum_counter = 0; // Reset quantum counter
        }
    }

    // If CPU is idle and ready queue is not empty
    if (*cpu_task == NULL && rq->head != NULL) {
        *cpu_task = dequeue_pcb(rq); // Get next task
        if (*cpu_task) {
            (*cpu_task)->status = TASK_RUNNING; // Set task status
            DBG("Process %d scheduled on CPU at %d ms\n", (*cpu_task)->pid, current_time_ms);
            quantum_counter = 0; // Reset quantum counter for new task
        }
    }
}
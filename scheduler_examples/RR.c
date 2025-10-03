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


//Resumo:
//Quantum define a "fatia de tempo" justa para cada processo.
//Se o processo termina dentro do quantum → é removido.
//Se não termina dentro do quantum → volta para o fim da fila.
//CPU nunca fica ociosa enquanto houver processos prontos.

#include "msg.h"
#include "queue.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define TICKS_MS    1      // Cada tick equivale a 1 ms (tempo simulado)
#define QUANTUM_MS  10     // Quantum = tempo máximo que um processo pode rodar de uma vez (10ms aqui)

// Função de escalonamento Round-Robin
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Contador estático que acumula quanto tempo o processo atual já rodou no quantum
    static uint32_t quantum_counter = 0;

    // Verifica se existe um processo em execução na CPU
    if (*cpu_task) {
        // Incrementa o tempo total que o processo já rodou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        // Incrementa o tempo rodado no quantum atual
        quantum_counter += TICKS_MS;

        // Caso 1: o processo terminou (tempo de execução >= tempo requerido)
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Prepara mensagem para notificar que o processo acabou
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            // Envia mensagem ao processo pela sua socket
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            // Liberta a memória do processo finalizado
            free(*cpu_task);
            *cpu_task = NULL;
            quantum_counter = 0; // reinicia contador de quantum
        }
        // Caso 2: o quantum expirou mas o processo ainda não terminou
        else if (quantum_counter >= QUANTUM_MS) {
            // Reinsere o processo no fim da fila de prontos
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;     // CPU fica livre
            quantum_counter = 0;  // reinicia contador de quantum
        }
    }

    // Se não há processo em execução e a fila não está vazia
    if (*cpu_task == NULL && rq->head != NULL) {
        // Remove o próximo processo da fila
        pcb_t *next_pcb = dequeue_pcb(rq);
        if (next_pcb) {
            // Coloca esse processo na CPU
            *cpu_task = next_pcb;
        }
    }
}

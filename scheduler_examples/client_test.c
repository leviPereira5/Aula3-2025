#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"   // define msg_t, PROCESS_REQUEST_RUN, etc.

int main() {
    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/scheduler.sock"); // ou SOCKET_PATH

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    // Enviar um RUN de 25ms
    msg_t msg = {
        .pid = 1,
        .request = PROCESS_REQUEST_RUN,
        .time_ms = 25
    };
    write(fd, &msg, sizeof(msg));

    // Esperar pelo ACK
    msg_t ack;
    read(fd, &ack, sizeof(ack));
    printf("ACK recebido: pid=%d, time=%d\n", ack.pid, ack.time_ms);

    // Depois podes ler DONE ou BLOCK, etc.
    while (read(fd, &ack, sizeof(ack)) > 0) {
        printf("Msg recebida: request=%d pid=%d time=%d\n",
               ack.request, ack.pid, ack.time_ms);
    }

    close(fd);
    return 0;
}

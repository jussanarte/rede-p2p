#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tcp_client.h"

int send_LNK(const char* ip, int port, int mySeq) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket TCP");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "LNK %d\n", mySeq);
    write(fd, msg, strlen(msg));

    char resp[64];
    int n = read(fd, resp, sizeof(resp)-1);
    resp[n] = '\0';

    printf("[TCP] Resposta do peer: %s\n", resp);

    close(fd);

    if (strncmp(resp, "CNF", 3) == 0)
        return 0;

    return -1;
}


int send_FRC(const char* ip, int port, int mySeq) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket TCP");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "FRC %d\n", mySeq);
    write(fd, msg, strlen(msg));

    char resp[64];
    int n = read(fd, resp, sizeof(resp)-1);
    if (n <= 0) {
        close(fd);
        return -1;
    }
    resp[n] = '\0';

    printf("[TCP] Resposta FRC: %s\n", resp);
    close(fd);

    if (strncmp(resp, "CNF", 3) == 0)
        return 0;

    return -1;
}

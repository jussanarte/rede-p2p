#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "udp_client.h"
char SERVER_IP[32] = "192.168.56.21";
int SERVER_UDP_PORT = 58000;


// Função interna para enviar e receber mensagens UDP
static int udp_send_recv(const char* msg, char* out, int outsize) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_UDP_PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Enviar pedido
    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr*)&servaddr, sizeof(servaddr));

    // Esperar resposta
    socklen_t len = sizeof(servaddr);
    int n = recvfrom(sockfd, out, outsize - 1, 0,
                     (struct sockaddr*)&servaddr, &len);

    if (n < 0) {
        perror("recvfrom");
        close(sockfd);
        return -1;
    }

    out[n] = '\0';
    close(sockfd);
    return n;
}

// ===================== REG =====================
int do_REG(int tcpPort) {
    char req[64], resp[128];
    snprintf(req, sizeof(req), "REG %d", tcpPort);

    int n = udp_send_recv(req, resp, sizeof(resp));
    if (n <= 0) return -1;

    int seq = -1;
    sscanf(resp, "SQN %d", &seq);

    return seq;  // retorna seqnumber ou -1
}

// ===================== PEERS =====================
int do_PEERS(char* buffer, int size) {
    return udp_send_recv("PEERS", buffer, size);
}

// ===================== UNR =====================
int do_UNR(int seq) {
    char req[64], resp[64];
    snprintf(req, sizeof(req), "UNR %d", seq);

    int n = udp_send_recv(req, resp, sizeof(resp));
    if (n <= 0) return -1;

    return (strncmp(resp, "OK", 2) == 0) ? 0 : -1;
}

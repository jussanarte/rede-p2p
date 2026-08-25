#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "tcp_server.h"
#include "graph.h"
#include "udp_client.h"

extern graph* rede;
extern peer* eu;
extern int Nplus;

void processPeersList(const char* resposta);

int start_tcp_server(int lnkport) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket TCP");
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(lnkport);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 10) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    printf("[TCP] Servidor ativo na porta %d\n", lnkport);
    return fd;
}

////////////////////////
int tcp_server_loop(int listen_fd) {

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    int conn = accept(listen_fd, (struct sockaddr*)&cliaddr, &len);
    if (conn < 0) {
        perror("accept");
        return 0;
    }

    char buffer[128];
    int n = read(conn, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(conn);
        return 0;
    }

    buffer[n] = '\0';
    printf("[TCP] Recebido: %s", buffer);
    fflush(stdout);

    int seq;

    /* =======================================================
       LNK seq
       ======================================================= */
    if (sscanf(buffer, "LNK %d", &seq) == 1) {

        if (eu->numInternos >= Nplus) {
            printf("[TCP] LNK recusado: internos cheios (%d/%d)\n",
                   eu->numInternos, Nplus);
            fflush(stdout);
            close(conn);
            return seq;
        }

        peer* remoto = findPeer(rede, seq);

        // Aceita ligação
        write(conn, "CNF\n", 4);
        printf("[TCP] Ligação confirmada com peer seq=%d\n", seq);
        fflush(stdout);

        if (remoto && eu)
            addLigacao(remoto, eu);

        close(conn);
        return seq;
    }

    /* =======================================================
       FRC seq
       ======================================================= */
    if (sscanf(buffer, "FRC %d", &seq) == 1) {
        printf("[TCP] Recebido FRC %d\n", seq);
        fflush(stdout);

        peer* remoto = findPeer(rede, seq);

        // Caso A: ainda há espaço → igual ao LNK
        if (eu->numInternos < Nplus) {
            write(conn, "CNF\n", 4);
            printf("[TCP] FRC aceite (tinha espaço)\n");
            fflush(stdout);

            if (remoto && eu)
                addLigacao(remoto, eu);

            close(conn);
            return seq;
        }

        // Caso B: estou cheio → procurar vizinho interno com seq maior
        vizinho* v = eu->internos;
        vizinho* candidato = NULL;

        while (v) {
            if (v->seq > seq) {
                candidato = v;
                break;
            }
            v = v->next;
        }

        if (!candidato) {
            printf("[TCP] FRC recusado: sem vizinho com seq maior\n");
            fflush(stdout);
            close(conn);
            return seq;
        }

        // Aceita FRC e expulsa vizinho maior
        write(conn, "CNF\n", 4);
        printf("[TCP] FRC aceite → removendo vizinho seq=%d\n", candidato->seq);
        fflush(stdout);

        peer* exp = findPeer(rede, candidato->seq);
        if (exp)
            removeLigacao(exp, eu);

        if (remoto && eu)
            addLigacao(remoto, eu);

        close(conn);
        return seq;
    }

    /* =======================================================
       UNL seq
       ======================================================= */
    if (sscanf(buffer, "UNL %d", &seq) == 1) {
        printf("[TCP] Peer %d enviou UNL → removendo ligação...\n", seq);
        fflush(stdout);

        peer* remoto = findPeer(rede, seq);
        if (remoto && eu)
            removeLigacao(remoto, eu);

        close(conn);
        return seq;
    }

    // Mensagem desconhecida
    close(conn);
    return 0;
}




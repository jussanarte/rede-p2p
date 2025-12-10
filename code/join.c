#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "join.h"
#include "udp_client.c"
#include "tcp_client.c"
#include "tcp_server.c"
#include "TAD_GRAFO_PEERS.c"

// Variáveis vindas do p2pnet
extern graph* rede;
extern peer* eu;
extern int mySeq;
extern int LNKPORT;
extern int Nplus;
extern int listen_fd;

/* ==========================================================
   FUNÇÃO PARA LER LISTA "LST" E COLOCAR NO GRAFO
   ========================================================== */
void processPeersList(const char* resposta) {
    const char* ptr = resposta;

    // saltar a linha "LST\n"
    while (*ptr && *ptr != '\n') ptr++;
    if (*ptr == '\n') ptr++;

    // cada linha: ip:port#seq
    while (*ptr) {
        char linha[128];
        int i = 0;

        while (*ptr && *ptr != '\n' && i < 127)
            linha[i++] = *ptr++;
        linha[i] = '\0';

        if (*ptr == '\n') ptr++;
        if (linha[0] == '\0') continue;

        char ip[32];
        int port, seq;
        if (sscanf(linha, "%[^:]:%d#%d", ip, &port, &seq) == 3) {
            if (seq == mySeq) continue;
            if (!findPeer(rede, seq)) {
                addPeer(rede, seq, ip, port);
            }
        }
    }
}

/* ==========================================================
   FUNÇÃO JOIN COMPLETA (UDP + TCP + FRC)
   ========================================================== */
void join() {
    if (mySeq != -1) {
        printf("Já estás registado.\n");
        return;
    }

    printf("\n--- JOIN ---\n");

    /* 1) REG */
    mySeq = do_REG(LNKPORT);
    if (mySeq <= 0) {
        printf("Erro REG.\n");
        return;
    }
    printf("SEQ atribuído: %d\n", mySeq);

    // cria o meu peer na estrutura local
    eu = addPeer(rede, mySeq, "192.168.56.11", LNKPORT);

    /* 2) PEERS */
    char buffer[1024];
    int n = do_PEERS(buffer, sizeof(buffer));
    if (n <= 0 || strncmp(buffer, "LST", 3) != 0) {
        printf("Erro PEERS.\n");
        return;
    }

    printf("Lista recebida do servidor:\n%s\n", buffer);
    processPeersList(buffer);
    limparPeersDesconhecidos(rede, buffer);

    /* 3) TCP → tentar ligações (LNK + FRC no máximo 1 vez) */
    int conectados = 0;
    int frc_usado = 0;   // só podemos usar FRC uma vez por JOIN

    for (int i = 0; i < rede->numPeers && conectados < Nplus; i++) {
        peer* p = &rede->peers[i];

        if (p->seq == -1) continue;        // slot vazio
        if (p->seq >= mySeq) continue;     // só ligo a peers com seq menor
        if (p->tcpPort <= 0) continue;

        printf("\nTentando ligação a peer seq=%d (%s:%d)\n",
               p->seq, p->ip, p->tcpPort);

        /* 3.1) Primeiro tenta LNK normal */
        if (send_LNK(p->ip, p->tcpPort, mySeq) == 0) {
            printf("Ligação CNF \n");
            // Eu → externo / p → interno
            addLigacao(eu, p);
            conectados++;
            continue;
        }

        printf("Não aceitou ligação.\n");

        /* 3.2) Se LNK falhou e ainda não usámos FRC, tentar FRC com ESTE peer */
        if (!frc_usado) {
            printf("Tentativa FRC com peer seq=%d...\n", p->seq);

            if (send_FRC(p->ip, p->tcpPort, mySeq) == 0) {
                printf("Ligação FRC CNF\n");
                addLigacao(eu, p);
                conectados++;
                frc_usado = 1;
                continue;
            } else {
                printf("FRC não aceito.\n");
                frc_usado = 1;   // já gastámos a tentativa de FRC
            }
        }

        // se chegou aqui: nem LNK nem FRC funcionaram com este peer
        // tenta o próximo, se houver
    }

    /* 4) Iniciar servidor TCP para receber LNK/FRC/UNL dos outros */
    listen_fd = start_tcp_server(LNKPORT);
    if (listen_fd < 0) {
        printf("[ERRO] TCP servidor não iniciou!\n");
        return;
    }

    printf("\nJOIN Finalizado. Ligações externas estabelecidas: %d\n", conectados);
}

#include <stdio.h>
#include <string.h>
#include "headers.h"
#include "udp_client.c"
#include "tad_grafo_peers.c"

// Variáveis globais vindas de p2pnet.c
extern graph* rede;
extern peer* eu;
extern int mySeq;
extern int LNKPORT;
extern int Nplus;

// Lê a lista LST… e adiciona ao grafo
void processPeersList(const char* resposta) {
    const char* ptr = resposta;

    // pular "LST\n"
    while (*ptr && *ptr != '\n') ptr++;
    if (*ptr == '\n') ptr++;

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
            // evita adicionar o próprio peer (caso já exista)
            if (seq != mySeq) {
                if (findPeer(rede, seq) == NULL) {
                    addPeer(rede, seq, ip, port);
                }
            }
        }
    }
}



void join() {
    if (mySeq != -1) {
        printf("Ja estas registado na rede.\n");
        return;
    }

    printf("\n--- JOIN INICIADO ---\n");

    // (i) REGISTAR NO SERVIDOR UDP
    mySeq = do_REG(LNKPORT);
   
	//getchar(); // limpar \n



    if (mySeq <= 0) {
        printf("[ERRO] Falha ao registar no servidor.\n");
        return;
    }

    printf("SQN  %d\n", mySeq);

    // Criar o nó local no grafo
    eu = addPeer(rede, mySeq, "192.168.56.11", LNKPORT);

    // (ii) PEDIR LISTA DE PEERS
    char buffer[1024];
    int n = do_PEERS(buffer, sizeof(buffer));

    if (n <= 0 || strncmp(buffer, "LST", 3) != 0) {
        printf("[ERRO] Falha ao obter lista de peers.\n");
        return;
    }

printf("%s\n", buffer);
printf("------------------------\n");


    // Guardar no grafo
    processPeersList(buffer);

   
    // (iii) LIGAR A PEERS (DEIXADO PARA DEPOIS)
    // connect_to_peers();
}

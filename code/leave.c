#include <stdio.h>
#include "leave.h"
#include "TAD_GRAFO_PEERS.c"

extern graph* rede;
extern peer* eu;
extern int mySeq;

void leave() {
    if (mySeq == -1) {
        printf("Ainda não estás registado.\n");
        return;
    }

    printf("--- LEAVE INICIADO ---\n");

    // Remover do servidor
    if (do_UNR(mySeq) == 0)
        printf("[OK] Removido do servidor de peers.\n");
    else
        printf("[ERRO] Falha ao remover no servidor.\n");

    // Remover vizinhos internos
    while (eu->internos != NULL) {
        vizinho* v = eu->internos;
        peer* outro = findPeer(rede, v->seq);

        if (outro)
            removeLigacao(outro, eu);
        else
            removeVizinho(&eu->internos, v->seq);
    }

    // Remover vizinhos externos
    while (eu->externos != NULL) {
        vizinho* v = eu->externos;
        peer* outro = findPeer(rede, v->seq);

        if (outro)
            removeLigacao(eu, outro);
        else
            removeVizinho(&eu->externos, v->seq);
    }

    // Resetar estado local
    mySeq = -1;
    eu = NULL;

    // Opcional: limpar também o grafo inteiro
    rede->numPeers = 0;
 
}

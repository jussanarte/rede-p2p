#ifndef TAD_GRAFO_PEERS_H
#define TAD_GRAFO_PEERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TAD_GRAFO_PEERS.h"
#define MAX_VERTICES 100
#define MAX_PEERS 100


// Estrutura de vizinho (lista ligada)
struct Vizinho {
    int seq;
    char ip[16]; 
    int port;
    struct Vizinho* next;
};

// Estrutura de peer
struct Peer {
    int seq;                 // seqnumber (ID do peer)
    char ip[16];             // IP do peer
    int tcpPort;             // porta TCP
    vizinho* internos;       // lista de vizinhos internos
    vizinho* externos;       // lista de vizinhos externos
    int numInternos;         // contador de vizinhos internos
    int numExternos;         // contador de vizinhos externos
   
};

// Estrutura de grafo
struct Graph {
    int numPeers;               // número de peers ativos
    peer peers[MAX_VERTICES];   // array de peers
};



// Cria um novo vizinho
vizinho* createVizinho(int seq, const char* ip, int port) {
    vizinho* v = malloc(sizeof(vizinho));
    if (!v) return NULL;

    v->seq = seq;
    strncpy(v->ip, ip, 16);
    v->ip[15] = '\0';
    v->port = port;
    v->next = NULL;
    return v;
}

// Adiciona ligação: a -> b (a ganha externo, b ganha interno)
void addLigacao(peer* a, peer* b) {
    if (!a || !b) return;

    // externo de A → B
    vizinho* e = createVizinho(b->seq, b->ip, b->tcpPort);
    e->next = a->externos;
    a->externos = e;
    a->numExternos++;

    // interno de B → A
    vizinho* i = createVizinho(a->seq, a->ip, a->tcpPort);
    i->next = b->internos;
    b->internos = i;
    b->numInternos++;
}



int count_list(vizinho* v) {
    int n = 0;
    while (v) {
        n++;
        v = v->next;
    }
    return n;
}

// Remove vizinho de uma lista ligada
void removeVizinho(vizinho** head, int seq) {
    if (!head || !(*head)) return;

    vizinho* temp = *head;
    vizinho* prev = NULL;

    while (temp) {
        if (temp->seq == seq) {
            if (prev) prev->next = temp->next;
            else      *head = temp->next;
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

// Remove ligação entre dois peers: a <-> b
void removeLigacao(peer* a, peer* b) {
    if (!a || !b) return;

    // Remove b das listas de a
    int beforeA = a->numExternos + a->numInternos;
    removeVizinho(&a->externos, b->seq);
    removeVizinho(&a->internos, b->seq);

    // Remove a das listas de b
    int beforeB = b->numExternos + b->numInternos;
    removeVizinho(&b->externos, a->seq);
    removeVizinho(&b->internos, a->seq);

    // Recontar vizinhos (100% correto)
    a->numExternos = count_list(a->externos);
    a->numInternos = count_list(a->internos);
    b->numExternos = count_list(b->externos);
    b->numInternos = count_list(b->internos);

    printf("Ligação removida: %d <-> %d\n", a->seq, b->seq);
}


// Imprime vizinhos de um peer
void printVizinho(peer* p) {
    if (!p) return;

    printf("\nPeer %s:%d (Seq %d)\n", p->ip, p->tcpPort, p->seq);

    // INTERNOS
    printf("Internos:\n");
    if (!p->internos) {
        printf("   (nenhum)\n");
    } else {
        vizinho* tmp = p->internos;
        while (tmp) {
            printf("   %s:%d#%d\n", tmp->ip, tmp->port, tmp->seq);
            tmp = tmp->next;
        }
    }

    // EXTERNOS
    printf("Externos:\n");
    if (!p->externos) {
        printf("   (nenhum)\n");
    } else {
        vizinho* tmp = p->externos;
        while (tmp) {
            printf("   %s:%d#%d\n", tmp->ip, tmp->port, tmp->seq);
            tmp = tmp->next;
        }
    }

    printf("\n");
}

// Inicializa grafo
graph* createGraph(void) {
    graph* g = malloc(sizeof(graph));
    if (!g) {
        printf("[ERRO]: Falha na alocação de memória para o grafo.\n");
        return NULL;
    }

    g->numPeers = 0;
    for (int i = 0; i < MAX_VERTICES; i++) {
        g->peers[i].seq = -1;
        g->peers[i].internos = NULL;
        g->peers[i].externos = NULL;
        g->peers[i].numInternos = 0;
        g->peers[i].numExternos = 0;
    }

    return g;
}

// Adiciona um peer ao grafo
peer* addPeer(graph* g, int seq, const char* ip, int tcpPort) {
    if (!g || g->numPeers >= MAX_VERTICES) return NULL;

    peer* p = &g->peers[g->numPeers];
    p->seq = seq;
    strncpy(p->ip, ip, 16);
    p->ip[15] = '\0';
    p->tcpPort = tcpPort;
    p->internos = NULL;
    p->externos = NULL;
    p->numInternos = 0;
    p->numExternos = 0;

    g->numPeers++;
    return p;
}

// Busca peer pelo seqnumber
peer* findPeer(graph* g, int seq) {
    if (!g) return NULL;
    for (int i = 0; i < g->numPeers; i++) {
        if (g->peers[i].seq == seq) return &g->peers[i];
    }
    return NULL;
}


void limparPeersDesconhecidos(graph* g, const char* lst) {
    for (int i = 0; i < g->numPeers; i++) {
        peer* p = &g->peers[i];

        if (p->seq == -1) continue;

        // Se o seq NÃO aparece no LST → remover do grafo
        char busca[32];
        snprintf(busca, sizeof(busca), "#%d", p->seq);

        if (strstr(lst, busca) == NULL) {
            // remover todos os vizinhos primeiro
            while (p->internos) removeVizinho(&p->internos, p->internos->seq);
            while (p->externos) removeVizinho(&p->externos, p->externos->seq);

            p->seq = -1; // marca como morto
        }
    }
}


#endif

#include <stdio.h>
#include "neighbors.h"
#include "graph.h"

extern peer* eu;

void show_neighbors() {

    printf("\n==== VIZINHOS INTERNOS ====\n");
    vizinho* v = eu->internos;

    if(!v) printf("(nenhum)\n");

    while(v) {
        printf("- IP: %s | PORTA: %d | SEQ: %d\n",
               v->ip, v->port, v->seq);
        v = v->next;
    }

    printf("\n==== VIZINHOS EXTERNOS ====\n");
    v = eu->externos;

    if(!v) printf("(nenhum)\n");

    while(v) {
        printf("- IP: %s | PORTA: %d | SEQ: %d\n",
               v->ip, v->port, v->seq);
        v = v->next;
    }

    printf("\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers.h"
#include "showneighbors.c"

void mostrarMenu() {
    printf("\n===== P2PNET =====\n");
    printf("> join\n");
    printf("> leave\n");
    printf("> show neighbors\n");
    printf("> exit\n");
    printf("> ");
}

void processarOpcao(char *opcao) {
	
    if (strcmp(opcao, "join") == 0) {
        join();
    } else if (strcmp(opcao, "leave") == 0) {
        leave();
    } else if (strcmp(opcao, "show neighbors") == 0) {
        if (eu == NULL) {
            printf("Ainda nao estas registado na rede.\n");
            return;
        }
        printVizinho(eu);
    }
    else if (strcmp(opcao, "exit") == 0) {
    	leave();
        exit(0);
    }
    else {
        printf("Opcao invalida.\n");
    }
}


